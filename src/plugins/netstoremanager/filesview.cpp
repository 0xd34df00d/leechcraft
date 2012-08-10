/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "filesview.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include <QAction>
#include <QDropEvent>
#include <QMenu>

namespace LeechCraft
{
namespace NetStoreManager
{
	FilesView::FilesView (QWidget *parent)
	: QTreeView (parent)
	, CurrentEvent_ (0)
	{
		CopyItem_ = new QAction ("Copy here", this);
		connect (CopyItem_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCopyItem ()));

		MoveItem_ = new QAction ("Move here", this);
		connect (MoveItem_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveItem ()));

		Cancel_ = new QAction ("Cancel", this);
		connect (Cancel_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCancel ()));
	}

	void FilesView::dropEvent (QDropEvent *event)
	{
		auto mimeData = event->mimeData ();
		QDataStream stream (mimeData->data ("application/nsm-item"));
		QString name;
		QStringList id, parentId;
		bool isInTrash = false, isDir = false;
		stream >> name
				>> id
				>> isInTrash
				>> isDir
				>> parentId;

		//TODO
		if (isInTrash ||
				id [0] == "netstoremanager.item_trash")
		{
			event->ignore ();
			return;
		}

		const auto& targetIndex = indexAt (event->pos ());
		if (!targetIndex.data (ListingRole::Directory).toBool () &&
				targetIndex.parent ().data (ListingRole::ID).toStringList () == parentId)
		{
			event->ignore ();
			return;
		}

		if (targetIndex.isValid ())
		{
			CurrentEvent_ = event;
			DraggedItemId_ = id;
			TargetItemId_ = targetIndex.data (ListingRole::Directory).toBool () ?
				targetIndex.data (ListingRole::ID).toStringList () :
				targetIndex.parent ().data (ListingRole::ID).toStringList ();

			QMenu *menu = new QMenu;

			if (!targetIndex.data (ListingRole::InTrash).toBool () &&
					targetIndex.data (ListingRole::ID).toStringList () [0] != "netstoremanager.item_trash");
			{
				!isDir ?
					menu->addActions ({ CopyItem_, MoveItem_, menu->addSeparator (), Cancel_ }) :
					menu->addActions ({ MoveItem_, menu->addSeparator (), Cancel_ });
			}

			menu->exec (viewport ()->mapToGlobal (event->pos ()));
			menu->deleteLater ();
		}
		else
			event->ignore ();

		CurrentEvent_ = 0;
		DraggedItemId_.clear ();
		TargetItemId_.clear ();
	}

	void FilesView::handleCopyItem ()
	{
		if (!CurrentEvent_ ||
				DraggedItemId_.isEmpty ())
			return;

		CurrentEvent_->setDropAction (Qt::CopyAction);
		emit copiedItem (DraggedItemId_, TargetItemId_);
		CurrentEvent_->accept ();
	}

	void FilesView::handleMoveItem ()
	{
		if (!CurrentEvent_ ||
				DraggedItemId_.isEmpty ())
			return;
		CurrentEvent_->setDropAction (Qt::MoveAction);
		emit movedItem (DraggedItemId_, TargetItemId_);
		CurrentEvent_->accept ();
	}

	void FilesView::handleCancel ()
	{
		if (CurrentEvent_)
			CurrentEvent_->ignore ();
	}
}
}

