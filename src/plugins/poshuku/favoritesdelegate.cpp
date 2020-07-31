/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "favoritesdelegate.h"
#include <util/tags/tagslineedit.h>
#include <util/tags/tagscompletionmodel.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "filtermodel.h"
#include "favoritesmodel.h"

namespace LC
{
namespace Poshuku
{
	using LC::Util::TagsCompleter;
	using LC::Util::TagsLineEdit;

	FavoritesDelegate::FavoritesDelegate (QObject *parent)
	: QItemDelegate (parent)
	{
	}

	QWidget* FavoritesDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& opt, const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
			return QItemDelegate::createEditor (parent, opt, index);

		TagsLineEdit *tle = new TagsLineEdit (parent);
		TagsCompleter_.reset (new TagsCompleter (tle));
		tle->AddSelector ();
		return tle;
	}

	void FavoritesDelegate::setEditorData (QWidget *editor,
			const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
		{
			QItemDelegate::setEditorData (editor, index);
			return;
		}

		auto itm = Core::Instance ().GetProxy ()->GetTagsManager ();
		auto tags = Core::Instance ().GetFavoritesModel ()->data (index, RoleTags).toStringList ();
		static_cast<TagsLineEdit*> (editor)->setText (itm->Join (itm->GetTags (tags)));
	}

	void FavoritesDelegate::setModelData (QWidget *editor,
			QAbstractItemModel *model, const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
		{
			QItemDelegate::setModelData (editor, model, index);
			return;
		}

		auto tags = Core::Instance ().GetProxy ()->GetTagsManager ()->Split (static_cast<TagsLineEdit*> (editor)->text ());
		model->setData (index, tags);
	}

	void FavoritesDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option,
			const QModelIndex& index) const
	{
		if (index.column () != FavoritesModel::ColumnTags)
		{
			QItemDelegate::updateEditorGeometry (editor, option, index);
			return;
		}

		editor->setGeometry (option.rect);
	}
}
}
