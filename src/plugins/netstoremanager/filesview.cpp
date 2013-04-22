/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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
#include <QAction>
#include <QDropEvent>
#include <QMenu>
#include "interfaces/netstoremanager/isupportfilelistings.h"

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
		QDataStream stream (mimeData->data ("x-leechcraft/nsm-item"));
		QString name;
		QByteArray id, parentId;
		bool isInTrash = false, isDir = false;
		struct ItemObject
		{
			QString name;
			QByteArray id;
			bool isInTrash;
			QByteArray parentId;
		};

		QList<ItemObject> items;
		while (!stream.atEnd ())
		{
			stream >> name
					>> id
					>> isInTrash
					>> isDir
					>> parentId;
			ItemObject item { name, id, isInTrash, parentId };
			items << item;
		}

		const auto& targetIndex = indexAt (event->pos ());

		if (targetIndex.data (ListingRole::ID).toByteArray () != "netstoremanager.item_trash" &&
				!targetIndex.data (ListingRole::InTrash).toBool ())
		{
			QList<QByteArray> ids;
			for (int i = items.count () - 1; i >= 0; --i)
			{
				if (items [i].isInTrash)
				{
					ids << items [i].id;
					items.removeAt (i);
				}
			}

			if (!ids.isEmpty ())
				emit itemsAboutToBeRestoredFromTrash (ids);
		}

		if (targetIndex.data (ListingRole::ID).toString () == "netstoremanager.item_trash" ||
				targetIndex.data (ListingRole::InTrash).toBool ())
		{
			QList<QByteArray> ids;
			for (int i = items.count () - 1; i >= 0; --i)
			{
				if (!items [i].isInTrash)
				{
					ids << items [i].id;
					items.removeAt (i);
				}
			}

			if (!ids.isEmpty ())
				emit itemsAboutToBeTrashed (ids);
		}

		if (!targetIndex.data (ListingRole::IsDirectory).toBool ())
		{
			QList<QByteArray> ids;
			for (int i = items.count () - 1; i >= 0; --i)
				if (items [i].parentId == targetIndex.parent ().data (ListingRole::ID).toByteArray ())
					items.removeAt (i);

			if (items.isEmpty ())
			{
				event->ignore ();
				return;
			}
		}

		if (targetIndex.isValid () &&
				!items.isEmpty ())
		{
			CurrentEvent_ = event;
			for (const auto& item : items)
				DraggedItemsIds_ << item.id;
			TargetItemId_ = targetIndex.data (ListingRole::IsDirectory).toBool () ?
				targetIndex.data (ListingRole::ID).toByteArray () :
				targetIndex.parent ().data (ListingRole::ID).toByteArray ();

			QMenu *menu = new QMenu;

			if (!targetIndex.data (ListingRole::InTrash).toBool () &&
					targetIndex.data (ListingRole::ID).toByteArray () != "netstoremanager.item_trash")
				menu->addActions ({ CopyItem_, MoveItem_, menu->addSeparator (), Cancel_ });

			menu->exec (viewport ()->mapToGlobal (event->pos ()));
			menu->deleteLater ();
		}
		else
			event->ignore ();

		CurrentEvent_ = 0;
		DraggedItemsIds_.clear ();
		TargetItemId_.clear ();
	}

	void FilesView::handleCopyItem ()
	{
		if (!CurrentEvent_ ||
				DraggedItemsIds_.isEmpty ())
			return;

		CurrentEvent_->setDropAction (Qt::CopyAction);
		emit itemsAboutToBeCopied (DraggedItemsIds_, TargetItemId_);
		CurrentEvent_->accept ();
	}

	void FilesView::handleMoveItem ()
	{
		if (!CurrentEvent_ ||
				DraggedItemsIds_.isEmpty ())
			return;
		CurrentEvent_->setDropAction (Qt::MoveAction);
		emit itemsAboutToBeMoved (DraggedItemsIds_, TargetItemId_);
		CurrentEvent_->accept ();
	}

	void FilesView::handleCancel ()
	{
		if (CurrentEvent_)
			CurrentEvent_->ignore ();
	}
}
}

