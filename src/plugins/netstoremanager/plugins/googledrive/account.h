/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QUrl>
#include <interfaces/netstoremanager/istorageaccount.h>
#include <interfaces/netstoremanager/isupportfilelistings.h>
#include "drivemanager.h"

namespace LC
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class Account;
	class Syncer;

	typedef std::shared_ptr<Account> Account_ptr;

	class Account : public QObject
					, public IStorageAccount
					, public ISupportFileListings
	{
		Q_OBJECT
		Q_INTERFACES (LC::NetStoreManager::IStorageAccount
				LC::NetStoreManager::ISupportFileListings)


		QObject *ParentPlugin_;
		QString Name_;
		QString AccessToken_;
		QString RefreshToken_;

		bool Trusted_ = false;

		DriveManager *DriveManager_;
		QHash<QString, DriveItem> Items_;

	public:
		Account (const QString& name, QObject *parentPlugin = 0);

		QObject* GetQObject ();
		QObject* GetParentPlugin () const;
		QByteArray GetUniqueID () const;
		AccountFeatures GetAccountFeatures () const;
		QString GetAccountName () const;
		void Upload (const QString& filepath,
				const QByteArray& parentId = QByteArray (),
				UploadType ut = UploadType::Upload,
				const QByteArray& id = QByteArray ());
		void Download (const QByteArray& id, const QString& filepath,
				TaskParameters tp, bool open);
		void DownloadFile (const QUrl& url, const QString& filepath,
				TaskParameters tp, bool open = false);

		ListingOps GetListingOps () const;
		HashAlgorithm GetCheckSumAlgorithm () const;

		QFuture<RefreshResult_t> RefreshListing ();
		void RefreshChildren (const QByteArray& parentId);

		void Delete (const QList<QByteArray>& ids, bool ask = true);
		void MoveToTrash (const QList<QByteArray>& ids);
		void RestoreFromTrash (const QList<QByteArray>& ids);
		void Copy (const QList<QByteArray>& ids, const QByteArray& newParentId);
		void Move (const QList<QByteArray>& ids, const QByteArray& newParentId);

		QFuture<RequestUrlResult_t> RequestUrl (const QByteArray& id);

		void CreateDirectory (const QString& name, const QByteArray& parentId);
		void Rename (const QByteArray& id, const QString& newName);
		void RequestChanges ();

		QByteArray Serialize ();
		static Account_ptr Deserialize (const QByteArray& data, QObject *parentPlugin);

		bool IsTrusted () const;
		void SetTrusted (bool trust);

		void SetAccessToken (const QString& token);
		void SetRefreshToken (const QString& token);
		QString GetRefreshToken () const;

		DriveManager* GetDriveManager () const;
	private slots:
		void handleGotNewItem (const DriveItem& item);
		void handleGotChanges (const QList<DriveChanges>& changes);
	signals:
		void upError (const QString& error, const QString& filepath);
		void upFinished (const QByteArray& id, const QString& filepath);
		void upProgress (quint64 done, quint64 total, const QString& filepath);
		void upStatusChanged (const QString& status, const QString& filepath);

		void listingUpdated (const QByteArray& parentId);

		void gotChanges (const QList<Change>& changes);

		void gotNewItem (const StorageItem& item, const QByteArray& parentId);

		void downloadFile (const QUrl& url, const QString& filePath,
			TaskParameters tp, bool open);
	};
}
}
}

