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

#ifndef PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEPLUGIN_H
#define PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEPLUGIN_H
#include <QString>
#include <QMetaType>
#include <QtPlugin>

class QIcon;

namespace LeechCraft
{
namespace NetStoreManager
{
	class IStoragePlugin
	{
	public:
		virtual ~IStoragePlugin () {}

		virtual QObject* GetObject () = 0;

		virtual QString GetStorageName () const = 0;
		virtual QIcon GetStorageIcon () const = 0;

		virtual void RegisterAccount (const QString&) = 0;
		virtual QObjectList GetAccounts () const = 0;
		virtual void RemoveAccount (QObject*) = 0;
	protected:
		virtual void accountAdded (QObject*) = 0;
		virtual void accountRemoved (QObject*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::NetStoreManager::IStoragePlugin,
		"org.Deviant.LeechCraft.NetStoreManager.IStoragePlugin/1.0");
Q_DECLARE_METATYPE (LeechCraft::NetStoreManager::IStoragePlugin*);

#endif
