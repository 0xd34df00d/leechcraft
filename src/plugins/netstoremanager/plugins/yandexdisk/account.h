/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <interfaces/netstoremanager/istorageaccount.h>

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
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::NetStoreManager::IStorageAccount)

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
		AccountFeatures GetAccountFeatures () const;
		void Upload (const QString&);
	};
}
}
}

#endif
