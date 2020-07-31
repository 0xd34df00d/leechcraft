/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "connectorbase.h"
#include <algorithm>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QtDebug>

namespace LC
{
namespace Liznoo
{
	ConnectorBase::ConnectorBase (const QString& service,
			const QByteArray& context, QObject* parent)
	: QObject { parent }
	, SB_ { QDBusConnection::connectToBus (QDBusConnection::SystemBus,
				"LeechCraft.Liznoo." + context + ".Connector") }
	, Service_ { service }
	{
	}

	bool ConnectorBase::TryAutostart ()
	{
		auto iface = SB_.interface ();
		auto checkRunning = [&iface, this]
		{
			return !iface->registeredServiceNames ()
					.value ().filter (Service_).isEmpty ();
		};
		if (checkRunning ())
			return true;

		iface->startService (Service_);
		if (checkRunning ())
			return true;

		qWarning () << Q_FUNC_INFO
				<< "failed to autostart"
				<< Service_;
		return false;
	}

	bool ConnectorBase::CheckSignals (const QString& path, const QStringList& signalsList)
	{
		const auto& introspect = QDBusInterface
		{
			Service_,
			path,
			"org.freedesktop.DBus.Introspectable",
			SB_
		}.call ("Introspect").arguments ().value (0).toString ();
		return std::all_of (signalsList.begin (), signalsList.end (),
				[&introspect] (const QString& signal) { return introspect.contains (signal); });
	}

	bool ConnectorBase::ArePowerEventsAvailable () const
	{
		return PowerEventsAvailable_;
	}

}
}
