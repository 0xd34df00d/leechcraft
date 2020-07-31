/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariant>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "syncwidget.h"

typedef QList<LC::NetStoreManager::Change> Changes_t;
Q_DECLARE_METATYPE (Changes_t)

class QThread;

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;
	class FilesWatcherBase;
	class Syncer;

	typedef QHash<QByteArray, Change> Snapshot_t;

	class SyncManager : public QObject
	{
		Q_OBJECT

		AccountsManager *AM_;
		FilesWatcherBase *FilesWatcher_;
		QHash<QString, Syncer*> AccountID2Syncer_;
		QHash<Syncer*, QThread*> Syncer2Thread_;

	public:
		SyncManager (AccountsManager *am, QObject *parent = 0);

		void Release ();
	private:
		Syncer* CreateSyncer (IStorageAccount *isa, const QString& baseDir,
				const QString& remoteDir);
		void WriteSnapshots ();
		void ReadSnapshots ();
		Syncer* GetSyncerByID (const QByteArray& id) const;
		Syncer* GetSyncerByLocalPath (const QString& localPath) const;

	public slots:
		void handleDirectoriesToSyncUpdated (const QList<SyncerInfo>& map);

	private slots:
		void handleDirWasCreated (const QString& path);
		void handleDirWasRemoved (const QString& path);
		void handleFileWasCreated (const QString& path);
		void handleFileWasRemoved (const QString& path);
		void handleFileWasUpdated (const QString& path);
		void handleEntryWasMoved (const QString& oldPath, const QString& newPath);
		void handleEntryWasRenamed (const QString& oldName, const QString& newName);

		void handleGotListing (const QList<StorageItem>& items);
		void handleGotNewItem (const StorageItem& item, const QByteArray& parentId);
		void handleGotChanges (const QList<Change>& changes);
	};
}
}
