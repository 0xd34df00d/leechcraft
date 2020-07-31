/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "logindconnector.h"
#include <QDBusInterface>
#include <QtDebug>

#include <unistd.h>

namespace LC
{
namespace Liznoo
{
namespace Logind
{
	LogindConnector::LogindConnector (QObject *parent)
	: QObject { parent }
	{
		QDBusInterface iface
		{
			"org.freedesktop.login1",
			"/org/freedesktop/login1",
			"org.freedesktop.DBus.Introspectable",
			SB_
		};
		if (!iface.isValid ())
			return;

		const auto& introspect = iface.call ("Introspect").arguments ().value (0).toString ();
		if (!introspect.contains ("\"PreparingForSleep\""))
		{
			qWarning () << Q_FUNC_INFO
					<< "no PreparingForSleep for logind";
			return;
		}

		SB_.connect ("org.freedesktop.login1",
				"/org/freedesktop/login1",
				"org.freedesktop.login1.Manager",
				"PreparingForSleep",
				this,
				SLOT (handlePreparing (bool)));
		PowerEventsAvailable_ = true;
	}

	bool LogindConnector::ArePowerEventsAvailable () const
	{
		return PowerEventsAvailable_;
	}

	void LogindConnector::Inhibit ()
	{
		const auto& value = QDBusInterface
		{
			"org.freedesktop.login1",
			"/org/freedesktop/login1",
			"org.freedesktop.login1.Manager"
		}.call ("Inhibit",
				"shutdown:sleep",
				"LeechCraft",
				tr ("Preparing LeechCraft for going to sleep..."),
				"delay");
		qDebug () << value;

		const auto fdVar = value.arguments ().value (0);
		qDebug () << fdVar;

		if (fdVar.canConvert<int> ())
			close (fdVar.toInt ());
	}

	void LogindConnector::handlePreparing (bool goingDown)
	{
		if (goingDown)
		{
			Inhibit ();
			emit gonnaSleep (5000);
		}
		else
			emit wokeUp ();
	}
}
}
}
