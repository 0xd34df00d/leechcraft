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

#include "todolistdelegate.h"
#include <QAbstractItemView>
#include <QStyle>
#include <util/tags/tagslineedit.h>
#include <util/tags/tagscompleter.h>
#include "storagemodel.h"

namespace LeechCraft
{
namespace Otlozhu
{
	TodoListDelegate::TodoListDelegate (QAbstractItemView *parent)
	: QStyledItemDelegate (parent)
	, View_ (parent)
	{
	}

	QWidget* TodoListDelegate::createEditor (QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case StorageModel::Columns::Tags:
		{
			auto edit = new Util::TagsLineEdit (parent);
			new Util::TagsCompleter (edit, edit);
			edit->AddSelector ();
			edit->setText (index.data (Qt::EditRole).toString ());
			return edit;
		}
		default:
			return QStyledItemDelegate::createEditor (parent, option, index);
		}
	}

	void TodoListDelegate::updateEditorGeometry (QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case StorageModel::Columns::Tags:
			editor->setGeometry (option.rect);
			break;
		default:
			QStyledItemDelegate::updateEditorGeometry (editor, option, index);
			break;
		}
	}

	void TodoListDelegate::paint (QPainter *painter, const QStyleOptionViewItem& thatOption, const QModelIndex& index) const
	{
		QStyleOptionViewItem option (thatOption);

		if (index.data (StorageModel::Roles::ItemProgress).toInt () == 100)
			option.font.setStrikeOut (true);

		switch (index.column ())
		{
		case StorageModel::Columns::Percentage:
		{
			QStyleOptionProgressBarV2 pbo;
			pbo.rect = option.rect;
			pbo.minimum = 0;
			pbo.maximum = 100;
			pbo.progress = index.data ().toInt ();
			pbo.state = option.state;
			pbo.text = index.data ().toString () + '%';
			pbo.textVisible = true;
			View_->style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
			break;
		}
		default:
			QStyledItemDelegate::paint (painter, option, index);
			break;
		}
	}
}
}
