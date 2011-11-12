/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "mailtreedelegate.h"
#include <QPainter>
#include "mailtab.h"

namespace LeechCraft
{
namespace Snails
{
	MailTreeDelegate::MailTreeDelegate (QObject *parent)
	: QStyledItemDelegate (parent)
	{
	}

	void MailTreeDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& item, const QModelIndex& index) const
	{
		const bool isRead = index.data (MailTab::Roles::ReadStatus).toBool ();

		if (isRead)
			QStyledItemDelegate::paint (painter, item, index);
		else
		{
			QStyleOptionViewItemV4 newItem = item;
			newItem.font.setBold (true);
			QStyledItemDelegate::paint (painter, newItem, index);
		}
	}
}
}
