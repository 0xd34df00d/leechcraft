/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagseditdelegate.h"
#include <QApplication>
#include <interfaces/core/itagsmanager.h>
#include "tagscompleter.h"
#include "tagslineedit.h"

namespace LC::Util
{
	TagsEditDelegate::TagsEditDelegate (ITagsManager& itm, QObject *parent)
	: QStyledItemDelegate { parent }
	, ITM_ { itm }
	{
	}

	QWidget* TagsEditDelegate::createEditor (QWidget *parent, const QStyleOptionViewItem&, const QModelIndex& index) const
	{
		const auto edit = new TagsLineEdit { parent };
		new TagsCompleter { edit };
		edit->AddSelector ();
		edit->setFrame (false);
		setEditorData (edit, index);
		return edit;
	}

	void TagsEditDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		auto opt = option;
		initStyleOption (&opt, index);
		opt.text = ITM_.Join (index.data ().toStringList ());

		const auto style = opt.widget ? opt.widget->style () : QApplication::style ();
		style->drawControl (QStyle::CE_ItemViewItem, &opt, painter);
	}

	void TagsEditDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
	{
		const auto& tags = index.data (Qt::EditRole).toStringList ();
		dynamic_cast<TagsLineEdit*> (editor)->setText (ITM_.Join (tags));
	}

	void TagsEditDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
	{
		const auto& tags = ITM_.Split (dynamic_cast<TagsLineEdit*> (editor)->text ());
		model->setData (index, tags, Qt::EditRole);
	}
}
