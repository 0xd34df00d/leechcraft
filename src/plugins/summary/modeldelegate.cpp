/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "modeldelegate.h"
#include <QApplication>
#include <QStyle>
#include <interfaces/ijobholder.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
namespace Summary
{
	ModelDelegate::ModelDelegate (QObject *parent)
	: QStyledItemDelegate (parent)
	{
	}

	namespace
	{
		bool DrawProgress (QPainter *painter,
				const QStyleOptionViewItem& option, const QModelIndex& index)
		{
			qlonglong done = index.data (ProcessState::Done).toLongLong ();
			qlonglong total = index.data (ProcessState::Total).toLongLong ();
			if (done < 0 || total <= 0)
				return false;

			while (done > 1000 && total > 1000)
			{
				done /= 10;
				total /= 10;
			}

			QStyleOptionProgressBarV2 pbo;
			pbo.rect = option.rect;
			pbo.minimum = 0;
			pbo.maximum = total;
			pbo.progress = done;
			pbo.state = option.state;
			pbo.text = index.data ().toString ();
			pbo.textVisible = true;
			QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
			return true;
		}
	}

	void ModelDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.column () == 2)
		{
			auto rowRole = index.data (CustomDataRoles::RoleJobHolderRow)
					.value<JobHolderRow> ();
			if ((rowRole == JobHolderRow::DownloadProgress ||
						rowRole == JobHolderRow::ProcessProgress) &&
					DrawProgress (painter, option, index))
				return;
		}

		QStyledItemDelegate::paint (painter, option, index);
	}
}
}
