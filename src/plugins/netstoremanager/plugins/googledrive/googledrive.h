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

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/netstoremanager/istorageplugin.h>
#include "account.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class AuthManager;

	class Plugin : public QObject
				, public IInfo
				, public IPlugin2
				, public IHaveSettings
				, public IStoragePlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveSettings
				LeechCraft::NetStoreManager::IStoragePlugin)

		QList<Account_ptr> Accounts_;
		AuthManager *AuthManager_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QObject* GetObject ();
		QObjectList GetAccounts () const;
		QIcon GetStorageIcon () const;
		QString GetStorageName () const;
		void RegisterAccount (const QString& name);
		void RemoveAccount (QObject *accObj);
	private:
		void WriteAccounts ();
		void ReadAccounts ();

	private slots:
		void handleAuthSuccess (QObject *accObj);

	signals:
		void accountAdded (QObject *accObj);
		void accountRemoved (QObject *accObj);
		void gotEntity (LeechCraft::Entity e);
	};
}
}
}
