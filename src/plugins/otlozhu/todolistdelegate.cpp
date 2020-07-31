/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "todolistdelegate.h"
#include <QAbstractItemView>
#include <QStyle>
#include <QDateTimeEdit>
#include <util/tags/tagslineedit.h>
#include <util/tags/tagscompleter.h>
#include "storagemodel.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Otlozhu
{
	TodoListDelegate::TodoListDelegate (QAbstractItemView *parent)
	: QStyledItemDelegate (parent)
	, View_ (parent)
	{
	}

	QWidget* TodoListDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case StorageModel::Columns::Tags:
		{
			auto edit = new Util::TagsLineEdit (parent);
			new Util::TagsCompleter (edit);
			edit->AddSelector ();
			edit->setText (index.data (Qt::EditRole).toString ());
			edit->setFrame (false);
			return edit;
		}
		case StorageModel::Columns::DueDate:
		case StorageModel::Columns::Created:
		{
			auto edit = new QDateTimeEdit (parent);
			edit->setFrame (false);
			edit->setCalendarPopup (true);
			edit->setDateTime (index.data (Qt::EditRole).toDateTime ());
			return edit;
		}
		default:
			return QStyledItemDelegate::createEditor (parent, option, index);
		}
	}

	void TodoListDelegate::updateEditorGeometry (QWidget* editor,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
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

	void TodoListDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& thatOption, const QModelIndex& index) const
	{
		QStyleOptionViewItem option (thatOption);

		if (index.data (StorageModel::Roles::ItemProgress).toInt () == 100 &&
				XmlSettingsManager::Instance ().property ("DoneStrikeOut").toBool ())
			option.font.setStrikeOut (true);

		switch (index.column ())
		{
		case StorageModel::Columns::Percentage:
		{
			QStyleOptionProgressBar pbo;
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
