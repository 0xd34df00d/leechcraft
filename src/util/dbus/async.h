/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusConnection>
#include <QDBusPendingReply>
#include "dbusconfig.h"

namespace LC::Util::DBus
{
	struct Endpoint
	{
		QString Service;
		QString Path;
		QString Interface;
		QDBusConnection Conn;

		UTIL_DBUS_API QDBusPendingCall GetProperty (const QString& property) const;
		UTIL_DBUS_API QDBusPendingReply<QVariantMap> GetAllProperties () const;
	};

	UTIL_DBUS_API QDBusPendingCall StartService (const QDBusConnection& conn, const QString& name);
}
