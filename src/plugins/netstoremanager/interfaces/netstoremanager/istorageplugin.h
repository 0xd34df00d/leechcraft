/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEPLUGIN_H
#define PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEPLUGIN_H
#include <QString>
#include <QMetaType>
#include <QtPlugin>

class QIcon;

namespace LC
{
namespace NetStoreManager
{
	class IStoragePlugin
	{
	public:
		virtual ~IStoragePlugin () {}

		virtual QObject* GetQObject () = 0;

		virtual QString GetStorageName () const = 0;
		virtual QString GetStorageIconName () const = 0;

		virtual void RegisterAccount (const QString&) = 0;
		virtual QObjectList GetAccounts () const = 0;
		virtual void RemoveAccount (QObject*) = 0;
	protected:
		virtual void accountAdded (QObject*) = 0;
		virtual void accountRemoved (QObject*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::NetStoreManager::IStoragePlugin,
		"org.Deviant.LeechCraft.NetStoreManager.IStoragePlugin/1.0")
Q_DECLARE_METATYPE (LC::NetStoreManager::IStoragePlugin*)

#endif
