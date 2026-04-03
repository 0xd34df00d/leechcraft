/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "async.h"
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <util/sll/qtutil.h>

namespace LC::Util::DBus
{
	QDBusPendingCall GetProperty (const Endpoint& endpoint, const QString& property)
	{
		auto msg = QDBusMessage::createMethodCall (endpoint.Service,
				endpoint.Path,
				"org.freedesktop.DBus.Properties"_qs,
				"Get"_qs);
		msg << endpoint.Interface << property;
		return endpoint.Conn.asyncCall (msg);
	}

	QDBusPendingCall StartService (const QDBusConnection& conn, const QString& name)
	{
		auto msg = QDBusMessage::createMethodCall ("org.freedesktop.DBus"_qs,
				"/org/freedesktop/DBus"_qs,
				"org.freedesktop.DBus"_qs,
				"StartServiceByName"_qs);
		msg << name << 0u;
		return conn.asyncCall (msg);
	}
}
