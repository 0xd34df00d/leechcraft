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

#ifndef PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_ACCOUNT_H
#define PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_ACCOUNT_H
#include <memory>
#include <QObject>
#include <QUrl>
#include <interfaces/netstoremanager/istorageaccount.h>
#include <interfaces/netstoremanager/isupportfilelistings.h>
#include "flitem.h"

class QNetworkRequest;

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	class Plugin;
	class AuthManager;

	class Account;
	typedef std::shared_ptr<Account> Account_ptr;

	class Account : public QObject
				  , public IStorageAccount
				  , public ISupportFileListings
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::NetStoreManager::IStorageAccount
				LeechCraft::NetStoreManager::ISupportFileListings)

		Plugin *Plugin_;
		QString Name_;

		QString Login_;

		AuthManager *AM_;
	public:
		Account (Plugin*);

		QByteArray Serialize () const;
		static Account_ptr Deserialize (const QByteArray&, Plugin*);

		AuthManager* GetAuthManager () const;
		QString GetLogin () const;
		QString GetPassword ();

		bool ExecConfigDialog ();

		void SetAccountName (const QString&);
		QString GetAccountName () const;
		QObject* GetParentPlugin () const;
		QObject* GetObject ();
		AccountFeatures GetAccountFeatures () const;
		void Upload (const QString&);

		ListingOps GetListingOps () const;

		void RefreshListing ();
		QStringList GetListingHeaders () const;

		void Delete (const QList<QStringList>&);
		void Prolongate (const QList<QStringList>&);

		QNetworkRequest MakeRequest (const QUrl& = QUrl ()) const;
	private:
		void SimpleAction (const QString&, const QList<QStringList>&);
	private slots:
		void forceRefresh ();
		void handleFileList (const QList<FLItem>&);
	signals:
		void upStatusChanged (const QString&, const QString&);
		void upProgress (quint64, quint64, const QString&);
		void upError (const QString&, const QString&);
		void gotURL (const QUrl&, const QString&);

		void gotListing (const QList<QList<QStandardItem*>>&);
	};
}
}
}

#endif
