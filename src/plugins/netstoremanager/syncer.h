/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <boost/bimap.hpp>
#include <QObject>
#include <QQueue>
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "syncmanager.h"

namespace LC
{
namespace NetStoreManager
{
	class IStorageAccount;

	class Syncer : public QObject
	{
		Q_OBJECT

		QString LocalPath_;
		QString RemotePath_;
		bool Started_;
		IStorageAccount *Account_;
		ISupportFileListings *SFLAccount_;
		QHash<QByteArray, StorageItem> Id2Item_;
		boost::bimaps::bimap<QByteArray, QString> Id2Path_;
		QQueue<std::function<void (void)>> CallsQueue_;

		Snapshot_t Snapshot_;

	public:
		explicit Syncer (const QString& dirPath, const QString& remotePath,
				IStorageAccount *isa, QObject *parent = 0);

		QByteArray GetAccountID () const;
		QString GetLocalPath () const;
		QString GetRemotePath () const;

		Snapshot_t GetSnapshot () const;
		void SetSnapshot (const Changes_t& changes);

		bool IsStarted () const;
	private:
		void CreateRemotePath (const QStringList& path);
		void DeleteRemotePath (const QStringList& path);
		void RenameItem (const StorageItem& item, const QString& path);
		Snapshot_t CreateSnapshot ();
		Snapshot_t CreateDiffSnapshot (const Snapshot_t& newSnapshot,
				const Snapshot_t& oldSnapshot);

	public slots:
		void start ();
		void stop ();

		void handleGotItems (const QList<StorageItem>& items);
		void handleGotNewItem (const StorageItem& item, const QByteArray& parentId);
		void handleGotChanges (const QList<Change>& changes);

		void localDirWasCreated (const QString& path);
		void localDirWasRemoved (const QString& path);
		void localFileWasCreated (const QString& path);
		void localFileWasRemoved (const QString& path);
		void localFileWasUpdated (const QString& path);
		void localFileWasRenamed (const QString& oldName, const QString& newName);
	};
}
}
