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

#include "yandexdisk.h"
#include <QIcon>

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NetStoreManager.YandexDisk";
	}

	QString Plugin::GetName () const
	{
		return "NetStoreManager: Yandex.Disk";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the Yandex.Disk for NetStoreManager plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.NetStoreManager.Plugins.IStoragePlugin";
		return classes;
	}

	QString Plugin::GetStorageName () const
	{
		return tr ("Yandex.Disk");
	}

	void Plugin::initPlugin (QObject *proxy)
	{
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_netstoremanager_yandexdisk, LeechCraft::NetStoreManager::YandexDisk::Plugin);
