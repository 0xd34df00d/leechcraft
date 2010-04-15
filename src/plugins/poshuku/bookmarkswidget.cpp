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

#include "bookmarkswidget.h"
#include <QInputDialog>
#include <QMessageBox>
#include <plugininterface/flattofoldersproxymodel.h>
#include "core.h"
#include "favoritesdelegate.h"
#include "favoritesmodel.h"
#include "editbookmarkdialog.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			BookmarksWidget::BookmarksWidget (QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);

				FavoritesFilterModel_.reset (new FilterModel (this));
				FavoritesFilterModel_->setSourceModel (Core::Instance ().GetFavoritesModel ());
				FavoritesFilterModel_->setDynamicSortFilter (true);

				FlatToFolders_.reset (new Util::FlatToFoldersProxyModel (this));
				FlatToFolders_->SetTagsManager (Core::Instance ()
						.GetProxy ()->GetTagsManager ());
				handleGroupBookmarks ();
				XmlSettingsManager::Instance ()->RegisterObject ("GroupBookmarksByTags",
						this, "handleGroupBookmarks");

				Ui_.FavoritesView_->setItemDelegate (new FavoritesDelegate (this));
				Ui_.FavoritesView_->addAction (Ui_.ActionEditBookmark_);
				Ui_.FavoritesView_->addAction (Ui_.ActionChangeURL_);
				Ui_.FavoritesView_->addAction (Ui_.ActionDeleteBookmark_);
				connect (Ui_.FavoritesView_,
						SIGNAL (deleteSelected (const QModelIndex&)),
						this,
						SLOT (translateRemoveFavoritesItem (const QModelIndex&)));
			
				FavoritesFilterLineCompleter_.reset (
						new Util::TagsCompleter (Ui_.FavoritesFilterLine_, this)
						);
				Ui_.FavoritesFilterLine_->AddSelector ();
				connect (Ui_.FavoritesFilterLine_,
						SIGNAL (tagsChosen ()),
						this,
						SLOT (selectTagsMode ()));
				connect (Ui_.FavoritesFilterLine_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (updateFavoritesFilter ()));
				connect (Ui_.FavoritesFilterType_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (updateFavoritesFilter ()));
				connect (Ui_.FavoritesFilterCaseSensitivity_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (updateFavoritesFilter ()));

				QHeaderView *itemsHeader = Ui_.FavoritesView_->header ();
				QFontMetrics fm = fontMetrics ();
				itemsHeader->resizeSection (0,
						fm.width ("Average site title can be very big, it's also the "
							"most important part, so it's priority is the biggest."));
				itemsHeader->resizeSection (1,
						fm.width ("Average URL could be very very long, but we don't account this."));
				itemsHeader->resizeSection (2,
						fm.width ("Average tags list size should be like this."));
			}

			void BookmarksWidget::on_ActionEditBookmark__triggered ()
			{
				QModelIndex current = Ui_.FavoritesView_->
					selectionModel ()->currentIndex ();
				if (FlatToFolders_->GetSourceModel ())
					current = FlatToFolders_->MapToSource (current);
				if (!current.isValid ())
					return;

				QModelIndex source = FavoritesFilterModel_->mapToSource (current);

				EditBookmarkDialog dia (source, this);
				if (dia.exec () != QDialog::Accepted)
					return;

				FavoritesModel *model = Core::Instance ().GetFavoritesModel ();
				model->setData (source.sibling (source.row (),
							FavoritesModel::ColumnTitle),
						dia.GetTitle ());
				model->setData (source.sibling (source.row (),
							FavoritesModel::ColumnTags),
						dia.GetTags ());
			}

			void BookmarksWidget::on_ActionChangeURL__triggered ()
			{
				QModelIndex current = Ui_.FavoritesView_->
					selectionModel ()->currentIndex ();
				if (FlatToFolders_->GetSourceModel ())
					current = FlatToFolders_->MapToSource (current);
				if (!current.isValid ())
					return;

				QModelIndex source = FavoritesFilterModel_->mapToSource (current);

				QString title = source.sibling (source.row (),
						FavoritesModel::ColumnTitle).data ().toString ();

				QString currentURL = source.sibling (source.row (),
						FavoritesModel::ColumnURL).data ().toString ();

				bool ok = false;

				QString newURL = QInputDialog::getText (this,
						tr ("Change URL"),
						tr ("Enter new URL for<br />%1")
							.arg (title),
						QLineEdit::Normal,
						currentURL,
						&ok);

				if (!ok)
					return;

				if (newURL.isEmpty ())
					QMessageBox::critical (this,
							"LeechCraft",
							tr ("URL of a bookmark can't be empty."));

				Core::Instance ().GetFavoritesModel ()->ChangeURL (source, newURL);
			}

			void BookmarksWidget::on_ActionDeleteBookmark__triggered ()
			{
				QModelIndex current = Ui_.FavoritesView_->
					selectionModel ()->currentIndex ();
				if (!current.isValid ())
					return;

				translateRemoveFavoritesItem (current);
			}
			
			void BookmarksWidget::translateRemoveFavoritesItem (const QModelIndex& sourceIndex)
			{
				QModelIndex index = sourceIndex;
				if (FlatToFolders_->GetSourceModel ())
					index = FlatToFolders_->MapToSource (index);
				index = FavoritesFilterModel_->mapToSource (index);
				Core::Instance ().GetFavoritesModel ()->removeItem (index);
			}

			void BookmarksWidget::updateFavoritesFilter ()
			{
				int section = Ui_.FavoritesFilterType_->currentIndex ();
				QString text = Ui_.FavoritesFilterLine_->text ();
			
				switch (section)
				{
					case 1:
						FavoritesFilterModel_->setTagsMode (false);
						FavoritesFilterModel_->setFilterWildcard (text);
						break;
					case 2:
						FavoritesFilterModel_->setTagsMode (false);
						FavoritesFilterModel_->setFilterRegExp (text);
						break;
					case 3:
						FavoritesFilterModel_->setTagsMode (true);
						FavoritesFilterModel_->setFilterFixedString (text);
						break;
					default:
						FavoritesFilterModel_->setTagsMode (false);
						FavoritesFilterModel_->setFilterFixedString (text);
						break;
				}
			
				FavoritesFilterModel_->
					setFilterCaseSensitivity ((Ui_.FavoritesFilterCaseSensitivity_->
								checkState () == Qt::Checked) ? Qt::CaseSensitive :
							Qt::CaseInsensitive);
			}
			
			void BookmarksWidget::on_FavoritesView__activated (const QModelIndex& act)
			{
				QModelIndex index;
				if (FlatToFolders_->GetSourceModel ())
					index = FlatToFolders_->MapToSource (act);

				Ui_.ActionEditBookmark_->setEnabled (index.isValid ());
				Ui_.ActionChangeURL_->setEnabled (index.isValid ());
				Ui_.ActionDeleteBookmark_->setEnabled (index.isValid ());

				if (index.isValid ())
					Core::Instance ().NewURL (index.sibling (index.row (),
								FavoritesModel::ColumnURL).data ().toString ());
				else if (act.isValid ())
					for (int i = 0, size = FlatToFolders_->rowCount (act);
							i < size; ++i)
					{
						QModelIndex idx = FlatToFolders_->
							index (i, FavoritesModel::ColumnURL, act);
						Core::Instance ().NewURL (FlatToFolders_->
								MapToSource (idx).data ().toString ());
					}
			}
			
			void BookmarksWidget::on_OpenInTabs__released ()
			{
				for (int i = 0, size = FavoritesFilterModel_->rowCount (); 
						i < size; ++i)
					Core::Instance ().NewURL (FavoritesFilterModel_->
							index (i, FavoritesModel::ColumnURL).data ().toString ());
			}

			void BookmarksWidget::selectTagsMode ()
			{
				Ui_.FavoritesFilterType_->setCurrentIndex (3);
				updateFavoritesFilter ();
			}

			void BookmarksWidget::handleGroupBookmarks ()
			{
				if (XmlSettingsManager::Instance ()->
						property ("GroupBookmarksByTags").toBool ())
				{
					FlatToFolders_->SetSourceModel (FavoritesFilterModel_.get ());
					Ui_.FavoritesView_->setModel (FlatToFolders_.get ());
				}
				else
				{
					FlatToFolders_->SetSourceModel (0);
					Ui_.FavoritesView_->setModel (FavoritesFilterModel_.get ());
				}
			}
		};
	};
};

