/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <plugininterface/tagscompleter.h>
#include "favoritesmodel.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
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

				QString url = index.sibling (index.row (), FavoritesModel::ColumnURL)
					.data ().toString ();
				if (url.size () > 100)
					url = QString ("%1...").arg (url.left (97));
				Ui_.Label_->setText (url);

				Ui_.Title_->setText (index.sibling (index.row (), FavoritesModel::ColumnTitle)
						.data ().toString ());

				Ui_.Tags_->setText (index.sibling (index.row (), FavoritesModel::ColumnTags)
						.data ().toString ());
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
		};
	};
};

