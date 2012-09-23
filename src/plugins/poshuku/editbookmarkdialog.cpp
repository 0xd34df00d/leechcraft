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

#include "editbookmarkdialog.h"
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "favoritesmodel.h"
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
	EditBookmarkDialog::EditBookmarkDialog (const QModelIndex& index,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		new Util::TagsCompleter (Ui_.Tags_);
		Ui_.Tags_->AddSelector ();

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
