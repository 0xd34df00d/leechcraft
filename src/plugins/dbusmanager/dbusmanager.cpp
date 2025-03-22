/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dbusmanager.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"

namespace LC
{
namespace DBusManager
{
	void DBusManager::Init (ICoreProxy_ptr proxy)
	{
		Core::Instance ().SetProxy (proxy);
	}

	void DBusManager::SecondInit ()
	{
	}

	void DBusManager::Release ()
	{
		Core::Instance ().Release ();
	}

	QByteArray DBusManager::GetUniqueID () const
	{
		return "org.LeechCraft.DBusManager";
	}

	QString DBusManager::GetName () const
	{
		return "DBus Manager";
	}

	QString DBusManager::GetInfo () const
	{
		return tr ("General DBus support for LeechCraft.");
	}

	QStringList DBusManager::Provides () const
	{
		return { "dbus" };
	}

	QIcon DBusManager::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_dbusmanager, LC::DBusManager::DBusManager);
