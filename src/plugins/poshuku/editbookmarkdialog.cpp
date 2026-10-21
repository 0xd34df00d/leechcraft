/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editbookmarkdialog.h"
#include <util/tags/tagseditoraddon.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "favoritesmodel.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
	EditBookmarkDialog::EditBookmarkDialog (const QModelIndex& index,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		new Util::TagsEditorAddon { *Ui_.Tags_, *Core::Instance ().GetProxy ()->GetTagsManager () };

		auto getRoleValue = [&index] (FavoritesModel::Columns column)
		{
			return index.sibling (index.row (), column).data ().toString ();
		};

		Ui_.URL_->setText (getRoleValue (FavoritesModel::ColumnURL));
		Ui_.Title_->setText (getRoleValue (FavoritesModel::ColumnTitle));
		Ui_.Tags_->setText (getRoleValue (FavoritesModel::ColumnTags));
	}

	QString EditBookmarkDialog::GetURL () const
	{
		return Ui_.URL_->text ();
	}

	QString EditBookmarkDialog::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QStringList EditBookmarkDialog::GetTags () const
	{
		return Core::Instance ().GetProxy ()->GetTagsManager ()->
			Split (Ui_.Tags_->text ());
	}
}
}
