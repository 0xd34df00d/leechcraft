/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filesview.h"
#include <QAction>
#include <QDropEvent>
#include <QMimeData>
#include <QMenu>
#include "interfaces/netstoremanager/isupportfilelistings.h"

namespace LC
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
			if (id != "netstoremanager.item_uplevel")
			{
				ItemObject item { name, id, isInTrash, parentId };
				items << item;
			}
		}

		const auto& targetIndex = indexAt (event->position ().toPoint ());

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

			menu->exec (viewport ()->mapToGlobal (event->position ().toPoint ()));
			menu->deleteLater ();
		}
		else
			event->ignore ();

		CurrentEvent_ = 0;
		DraggedItemsIds_.clear ();
		TargetItemId_.clear ();
	}

	void FilesView::keyReleaseEvent (QKeyEvent *event)
	{
		switch (event->key ())
		{
		case Qt::Key_Return:
			emit returnPressed ();
			break;
		case Qt::Key_Backspace:
			emit backspacePressed ();
			break;
		case Qt::Key_QuoteLeft:
			emit quoteLeftPressed ();
			break;
		default:
			QWidget::keyReleaseEvent (event);
			break;
		}
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
