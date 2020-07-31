/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/netstoremanager/istorageplugin.h>
#include "account.h"

namespace LC
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
				LC::NetStoreManager::IStoragePlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.NetStoreManager.GoogleDrive")

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

		QObject* GetQObject ();
		QObjectList GetAccounts () const;
		QString GetStorageIconName () const;
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
	};
}
}
}
