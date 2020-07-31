/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncer.h"
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include "xmlsettingsmanager.h"
#include "drivemanager.h"

namespace LC
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	Syncer::Syncer (DriveManager *dm, QObject *parent)
	: QObject (parent)
	, LastChangesId_ (XmlSettingsManager::Instance ()
			.Property ("LastChangesId", 0).toLongLong ())
	, DM_ (dm)
	{
	}

	void Syncer::CheckRemoteStorage ()
	{
		connect (DM_,
				SIGNAL (gotChanges (QList<DriveChanges>, qlonglong)),
				this,
				SLOT (handleGotDriveChanges (QList<DriveChanges>, qlonglong)));
		DM_->RequestFileChanges (LastChangesId_);
	}

	void Syncer::CheckLocalStorage (const QStringList& paths,
			const QString& baseDir)
	{
		BaseDir_ = baseDir;
		Paths_ =  paths;
		std::sort (Paths_.begin (), Paths_.end ());
		connect (DM_,
				SIGNAL (gotFiles (QList<DriveItem>)),
				this,
				SLOT (handleGotFiles (QList<DriveItem>)));
		DM_->RefreshListing ();
	}

	void Syncer::ContinueLocalStorageChecking ()
	{
		if (Paths_.isEmpty ())
		{
			//TODO checking finished
			qDebug () << "finished";
			return;
		}

		QString path = Paths_.takeAt (0);
		QFileInfo info (path);
		DriveItem parentItem = RealPath2Item_.value (info.dir ().absolutePath ());

		bool found = false;
		DriveItem currentItem;
		for (const auto& item : Items_)
		{
			if (item.IsFolder_ != info.isDir () ||
					item.Name_ != info.fileName () ||
					item.Labels_ & DriveItem::ILRemoved ||
					item.ParentId_ != parentItem.Id_)
				continue;

			currentItem = item;
			found = true;
			Items_.removeOne (item);
			break;
		}

		if (found)
		{
			if (QFileInfo (path).exists ())
			{
				RealPath2Item_ [path] = currentItem;
				if (!info.isDir ())
				{
					QFile file (path);
					if (file.open (QIODevice::ReadOnly))
					{
						if (currentItem.Md5_ != QCryptographicHash::hash (file.readAll (), QCryptographicHash::Md5).toHex ())
						{
							//TODO update
							qDebug () << currentItem.Id_ << currentItem.Name_ << "need update";
						}
					}
				}
			}
			else
				DM_->MoveEntryToTrash (currentItem.Id_);
			ContinueLocalStorageChecking ();
		}
		else
		{
			connect (DM_,
					SIGNAL (gotNewItem (DriveItem)),
					this,
					SLOT (handleGotNewItem (DriveItem)));
			RealPathQueue_ << path;
			!info.isDir () ?
				DM_->Upload (path, QStringList () << parentItem.Id_) :
				DM_->CreateDirectory (info.fileName (), parentItem.Id_);
		}
	}

	void Syncer::handleGotDriveChanges (const QList<DriveChanges>& changes, qlonglong id)
	{
		disconnect (DM_,
			SIGNAL (gotChanges (QList<DriveChanges>, qlonglong)),
			this,
			SLOT (handleGotDriveChanges (QList<DriveChanges>, qlonglong)));

		XmlSettingsManager::Instance ().setProperty ("LastChangesId", id);
		LastChangesId_ = id;
	}

	void Syncer::handleGotFiles (const QList<DriveItem>& files)
	{
		Items_ = files;
		disconnect (DM_,
				SIGNAL (gotFiles (QList<DriveItem>)),
				this,
				SLOT (handleGotFiles (QList<DriveItem>)));

		const QString& rootName = QFileInfo (BaseDir_).fileName ();
		DriveItem rootItem;
		bool found = false;
		for (const auto& item : files)
		{
			if (!item.ParentIsRoot_ ||
					!item.IsFolder_ ||
					item.Name_ != rootName ||
					item.Labels_ & DriveItem::ILRemoved)
				continue;

			rootItem = item;
			found = true;
			Items_.removeOne (rootItem);
			break;
		}

		if (!found)
		{
			connect (DM_,
					SIGNAL (gotNewItem (DriveItem)),
					this,
					SLOT (handleGotNewItem (DriveItem)));
			RealPathQueue_ << BaseDir_;
			DM_->CreateDirectory (rootName);
		}
		else
		{
			if (QFileInfo (BaseDir_).exists ())
				RealPath2Item_ [BaseDir_] = rootItem;
			else
				DM_->MoveEntryToTrash (rootItem.Id_);

			ContinueLocalStorageChecking ();
		}
	}

	void Syncer::handleGotNewItem (const DriveItem& item)
	{
		disconnect (DM_,
				SIGNAL (gotNewItem (DriveItem)),
				this,
				SLOT (handleGotNewItem (DriveItem)));
		if (!RealPathQueue_.isEmpty ())
			RealPath2Item_ [RealPathQueue_.dequeue ()] = item;

		Items_ << item;
		qDebug () << "created entry: " << item.Name_ << RealPath2Item_.key (item);
		ContinueLocalStorageChecking ();
	}
}
}
}

