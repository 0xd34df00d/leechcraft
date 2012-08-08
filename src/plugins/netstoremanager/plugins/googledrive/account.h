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

#pragma once

#include <memory>
#include <QObject>
#include <QUrl>
#include <interfaces/netstoremanager/istorageaccount.h>
#include <interfaces/netstoremanager/isupportfilelistings.h>
#include "drivemanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class Account;

	typedef std::shared_ptr<Account> Account_ptr;

	class Account : public QObject
					, public IStorageAccount
					, public ISupportFileListings
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::NetStoreManager::IStorageAccount
				LeechCraft::NetStoreManager::ISupportFileListings)


		QObject *ParentPlugin_;
		QString Name_;
		QString AccessToken_;
		QString RefreshToken_;

		bool Trusted_;

		DriveManager *DriveManager_;
		QHash<QString, DriveItem> Items_;

	public:
		Account (const QString& name, QObject *parentPlugin = 0);

		QObject* GetObject ();
		QObject* GetParentPlugin () const;
		AccountFeatures GetAccountFeatures () const;
		QString GetAccountName () const;
		void Upload (const QString& filepath);

		void Delete (const QList<QStringList>& id);
		QStringList GetListingHeaders () const;
		ListingOps GetListingOps () const;
		void MoveToTrash (const QList<QStringList>& ids);
		void RestoreFromTrash (const QList<QStringList>& ids);
		void EmptyTrash (const QList<QStringList>& ids);
		void RefreshListing ();
		void RequestUrl (const QList<QStringList>& id);
		void CreateDirectory (const QString& name, const QStringList& parentId);

		QByteArray Serialize ();
		static Account_ptr Deserialize (const QByteArray& data, QObject *parentPlugin);

		bool IsTrusted () const;
		void SetTrusted (bool trust);

		void SetAccessToken (const QString& token);
		void SetRefreshToken (const QString& token);
		QString GetRefreshToken () const;

		DriveManager* GetDriveManager () const;
	private slots:
		void handleFileList (const QList<DriveItem>& items);
		void handleSharedFileId (const QString& id);

	signals:
		void gotURL (const QUrl& url, const QString& filepath);
		void upError (const QString& error, const QString& filepath);
		void upProgress (quint64 done, quint64 total, const QString& filepath);
		void upStatusChanged (const QString& status, const QString& filepath);

		void gotListing (const QList<QList<QStandardItem*>>& items);
		void gotFileUrl (const QUrl& url, const QStringList& id);
	};
}
}
}

