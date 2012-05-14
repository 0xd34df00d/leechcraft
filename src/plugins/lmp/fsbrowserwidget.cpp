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

#include "fsbrowserwidget.h"
#include <QDir>
#include "fsmodel.h"
#include "util.h"
#include "player.h"
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	FSBrowserWidget::FSBrowserWidget (QWidget *parent)
	: QWidget (parent)
	, Player_ (0)
	, FSModel_ (new FSModel (this))
	{
		Ui_.setupUi (this);

		FSModel_->setReadOnly (true);
		FSModel_->setRootPath (QDir::rootPath ());
		Ui_.FSTree_->setModel (FSModel_);

		QAction *addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromFSBrowser ()));
		Ui_.FSTree_->addAction (addToPlaylist);

		DirCollection_ = new QAction (QString (), this);
		DirCollection_->setProperty ("WatchActionIconChange", true);
		Ui_.FSTree_->addAction (DirCollection_);

		connect (Ui_.FSTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex)));

		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (rootPathsChanged (QStringList)),
				this,
				SLOT (handleCollectionChanged ()));
	}

	void FSBrowserWidget::AssociatePlayer (Player *player)
	{
		Player_ = player;
	}

	void FSBrowserWidget::handleItemSelected (const QModelIndex& index)
	{
		const auto& path = FSModel_->fileInfo (index).absoluteFilePath ();

		disconnect (DirCollection_,
				0,
				this,
				0);

		switch (Core::Instance ().GetLocalCollection ()->GetDirStatus (path))
		{
		case LocalCollection::DirStatus::None:
			DirCollection_->setText (tr ("Add to collection..."));
			DirCollection_->setEnabled (true);
			connect (DirCollection_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleAddToCollection ()));
			break;
		case LocalCollection::DirStatus::RootPath:
			DirCollection_->setText (tr ("Remove from collection..."));
			DirCollection_->setEnabled (true);
			connect (DirCollection_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleRemoveFromCollection ()));
			break;
		case LocalCollection::DirStatus::SubPath:
			DirCollection_->setText (tr ("Already in collection"));
			DirCollection_->setEnabled (false);
			break;
		}
	}

	void FSBrowserWidget::handleCollectionChanged ()
	{
		handleItemSelected (Ui_.FSTree_->currentIndex ());
	}

	void FSBrowserWidget::handleAddToCollection ()
	{
		const auto& index = Ui_.FSTree_->currentIndex ();
		const auto& path = FSModel_->fileInfo (index).absoluteFilePath ();

		Core::Instance ().GetLocalCollection ()->Scan (path);
	}

	void FSBrowserWidget::handleRemoveFromCollection ()
	{
		const auto& index = Ui_.FSTree_->currentIndex ();
		const auto& path = FSModel_->fileInfo (index).absoluteFilePath ();

		Core::Instance ().GetLocalCollection ()->Unscan (path);
	}

	void FSBrowserWidget::loadFromFSBrowser ()
	{
		if (!Player_)
			return;

		const QModelIndex& index = Ui_.FSTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const QFileInfo& fi = FSModel_->fileInfo (index);

		if (fi.isDir ())
			Player_->Enqueue (RecIterate (fi.absoluteFilePath ()));
		else
			Player_->Enqueue (QStringList (fi.absoluteFilePath ()));
	}
}
}
