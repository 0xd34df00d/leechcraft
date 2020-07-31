/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncmanager.h"
#include <QtDebug>
#include <QSettings>
#include <QThread>
#include "accountsmanager.h"
#include "syncer.h"
#if defined (Q_OS_LINUX)
	#include "fileswatcher_inotify.h"
#else
	#include "fileswatcher_dummy.h"
#endif
#include "syncwidget.h"

namespace LC
{
namespace NetStoreManager
{

	SyncManager::SyncManager (AccountsManager *am, QObject *parent)
	: QObject (parent)
	, AM_ (am)
	{
#if defined (Q_OS_LINUX)
		FilesWatcher_ = new FilesWatcherInotify (this);
#else
		FilesWatcher_ = new FilesWatcherDummy (this);
#endif

		connect (FilesWatcher_,
				SIGNAL (dirWasCreated (QString)),
				this,
				SLOT (handleDirWasCreated (QString)));
		connect (FilesWatcher_,
				SIGNAL (dirWasRemoved (QString)),
				this,
				SLOT (handleDirWasRemoved (QString)));
		connect (FilesWatcher_,
				SIGNAL (fileWasCreated (QString)),
				this,
				SLOT (handleFileWasCreated (QString)));
		connect (FilesWatcher_,
				SIGNAL (fileWasRemoved (QString)),
				this,
				SLOT (handleFileWasRemoved (QString)));
		connect (FilesWatcher_,
				SIGNAL (fileWasUpdated (QString)),
				this,
				SLOT (handleFileWasUpdated (QString)));
		connect (FilesWatcher_,
				SIGNAL (entryWasMoved (QString, QString)),
				this,
				SLOT (handleEntryWasMoved (QString, QString)));
		connect (FilesWatcher_,
				SIGNAL (entryWasRenamed (QString, QString)),
				this,
				SLOT (handleEntryWasRenamed (QString, QString)));

		for (auto account : AM_->GetAccounts ())
		{
// 			auto isfl = qobject_cast<ISupportFileListings*> (account->GetQObject ());
// 			if (!isfl)
// 				continue;
// 			connect (account->GetQObject (),
// 					SIGNAL (gotListing (QList<StorageItem>)),
// 					this,
// 					SLOT (handleGotListing (QList<StorageItem>)));
// 			connect (account->GetQObject (),
// 					SIGNAL (gotNewItem (StorageItem, QByteArray)),
// 					this,
// 					SLOT (handleGotNewItem (StorageItem, QByteArray)));
// 			connect (account->GetQObject (),
// 					SIGNAL (gotChanges (QList<Change>)),
// 					this,
// 					SLOT (handleGotChanges (QList<Change>)));
		}
	}

	void SyncManager::Release ()
	{
		for (auto syncer : Syncer2Thread_.keys ())
		{
			syncer->stop ();
			auto thread = Syncer2Thread_.value (syncer);
			thread->quit ();
		}

		for (auto syncer : Syncer2Thread_.keys ())
		{
			syncer->stop ();
			auto thread = Syncer2Thread_.take (syncer);
			if (!thread->isFinished () &&
					!thread->wait (3000))
				thread->terminate ();
			syncer->deleteLater ();
			thread->deleteLater ();
		}
	}

	void SyncManager::handleDirectoriesToSyncUpdated (const QList<SyncerInfo>& infos)
	{
		QStringList paths;
		for (const auto& info : infos)
		{
			paths << info.LocalDirectory_;
			auto acc = AM_->GetAccountFromUniqueID (info.AccountId_);

			if (AccountID2Syncer_.contains (info.AccountId_))
			{
				auto syncer = AccountID2Syncer_ [info.AccountId_];
				if (syncer->GetLocalPath () == info.LocalDirectory_ &&
						syncer->GetRemotePath () == info.RemoteDirectory_)
					continue;
				else
				{

					//TODO update syncer
// 					syncer->stop ();
// 					AccountID2Syncer_.take (info.AccountId_)->deleteLater ();
				}
			}
			else
			{
				auto syncer = CreateSyncer (acc, info.LocalDirectory_, info.RemoteDirectory_);
				AccountID2Syncer_ [info.AccountId_] = syncer;
// 				syncer->start ();
			}
		}

		FilesWatcher_->updatePaths (paths);
	}

	Syncer* SyncManager::CreateSyncer (IStorageAccount *isa,
			const QString& baseDir, const QString& remoteDir)
	{
		QThread *thread = new QThread (this);
		Syncer *syncer = new Syncer (baseDir, remoteDir, isa);
		syncer->moveToThread (thread);
		thread->start ();
		Syncer2Thread_ [syncer] = thread;

		return syncer;
	}

	void SyncManager::WriteSnapshots ()
	{
	}

	void SyncManager::ReadSnapshots ()
	{
	}

	Syncer* SyncManager::GetSyncerByID (const QByteArray& id) const
	{
		for (auto accountId : AccountID2Syncer_.keys ())
			if (id == accountId)
				return AccountID2Syncer_ [accountId];
		return 0;
	}

	Syncer* SyncManager::GetSyncerByLocalPath (const QString& localPath) const
	{
		for (auto syncer : AccountID2Syncer_)
			if (localPath.startsWith (syncer->GetLocalPath ()))
				return syncer;

		return 0;
	}

	void SyncManager::handleDirWasCreated (const QString& path)
	{
// 		if (auto syncer = GetSyncerByLocalPath (path))
// 			syncer->localDirWasCreated (path);
	}

	void SyncManager::handleDirWasRemoved (const QString& path)
	{
// 		if (auto syncer = GetSyncerByLocalPath (path))
// 			syncer->localDirWasRemoved (path);
	}

	void SyncManager::handleFileWasCreated (const QString& path)
	{
// 		if (auto syncer = GetSyncerByLocalPath (path))
// 			syncer->localFileWasCreated (path);
	}

	void SyncManager::handleFileWasRemoved (const QString& path)
	{
// 		if (auto syncer = GetSyncerByLocalPath (path))
// 			syncer->localFileWasRemoved (path);
	}

	void SyncManager::handleFileWasUpdated (const QString& path)
	{
// 		if (auto syncer = GetSyncerByLocalPath (path))
// 			syncer->localFileWasUpdated (path);
	}

	void SyncManager::handleEntryWasMoved (const QString& oldPath,
			const QString& newPath)
	{
		qDebug () << Q_FUNC_INFO << oldPath << newPath;
	}

	void SyncManager::handleEntryWasRenamed (const QString& oldName,
			const QString& newName)
	{
// 		if (auto syncer = GetSyncerByLocalPath (oldName))
// 			syncer->localFileWasRenamed (oldName, newName);
	}

	void SyncManager::handleGotListing (const QList<StorageItem>& items)
	{
// 		auto isa = qobject_cast<IStorageAccount*> (sender ());
// 		if (!isa)
// 			return;
//
// 		if (auto syncer = GetSyncerByID (isa->GetUniqueID ()))
// 		{
// 			syncer->handleGotItems (items);
// 			if (!syncer->IsStarted ())
// 				syncer->start ();
// 		}
	}

	void SyncManager::handleGotNewItem (const StorageItem& item,
			const QByteArray& parentId)
	{
// 		auto isa = qobject_cast<IStorageAccount*> (sender ());
// 		if (!isa)
// 			return;
//
// 		if (auto syncer = GetSyncerByID (isa->GetUniqueID ()))
// 			syncer->handleGotNewItem (item, parentId);
	}

	void SyncManager::handleGotChanges (const QList<Change>& changes)
	{
// 		auto isa = qobject_cast<IStorageAccount*> (sender ());
// 		if (!isa)
// 			return;
//
// 		if (auto syncer = GetSyncerByID (isa->GetUniqueID ()))
// 			syncer->handleGotChanges (changes);
	}

}
}

