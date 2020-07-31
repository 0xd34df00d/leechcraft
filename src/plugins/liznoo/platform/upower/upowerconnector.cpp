/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upowerconnector.h"
#include <QTimer>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QtDebug>
#include <util/xpc/util.h>

namespace LC
{
namespace Liznoo
{
namespace UPower
{
	UPowerConnector::UPowerConnector (QObject *parent)
	: ConnectorBase { "org.freedesktop.UPower", "UPower", parent }
	{
		if (!TryAutostart ())
			return;

		SB_.connect ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				"DeviceAdded",
				this,
				SLOT (requeryDevice (QString)));

		ConnectChangedNotification ();

		if (!CheckSignals ("/org/freedesktop/UPower", { "\"Sleeping\"", "\"Resuming\"" }))
		{
			qDebug () << Q_FUNC_INFO
					<< "no Sleeping() or Resuming() signals, we are probably on systemd";
			return;
		}

		const auto sleepConnected = SB_.connect ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				"Sleeping",
				this,
				SLOT (handleGonnaSleep ()));
		const auto resumeConnected = SB_.connect ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				"Resuming",
				this,
				SIGNAL (wokeUp ()));

		PowerEventsAvailable_ = sleepConnected && resumeConnected;
	}

	void UPowerConnector::ConnectChangedNotification ()
	{
		HasGlobalDeviceChanged_ = CheckSignals ("/org/freedesktop/UPower", { "DeviceChanged" });
		qDebug () << Q_FUNC_INFO
			<< "has global DeviceChanged signal?"
			<< HasGlobalDeviceChanged_;

		if (HasGlobalDeviceChanged_)
			SB_.connect ("org.freedesktop.UPower",
					"/org/freedesktop/UPower",
					"org.freedesktop.UPower",
					"DeviceChanged",
					this,
					SLOT (requeryDevice (QString)));
	}

	void UPowerConnector::handleGonnaSleep ()
	{
		emit gonnaSleep (1000);
	}

	void UPowerConnector::enumerateDevices ()
	{
		QDBusInterface face ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				SB_);

		auto res = face.call ("EnumerateDevices");
		for (const auto& argument : res.arguments ())
		{
			auto arg = argument.value<QDBusArgument> ();
			QList<QDBusObjectPath> paths;
			arg >> paths;
			for (const auto& path : paths)
				requeryDevice (path.path ());
		}
	}

	namespace
	{
		QString TechIdToString (int id)
		{
			switch (id)
			{
			case 1:
				return "Li-Ion";
			case 2:
				return "Li-Polymer";
			case 3:
				return "Li-Iron-Phosphate";
			case 4:
				return "Lead acid";
			case 5:
				return "NiCd";
			case 6:
				return "NiMh";
			default:
				qWarning () << Q_FUNC_INFO
					<< "unknown technology ID"
					<< id;
				return "<unknown>";
			}
		}
	}

	void UPowerConnector::requeryDevice (const QString& id)
	{
		QDBusInterface face ("org.freedesktop.UPower",
				id,
				"org.freedesktop.UPower.Device",
				SB_);
		if (face.property ("Type").toInt () != 2)
			return;

		BatteryInfo info;
		info.ID_ = id;
		info.Percentage_ = face.property ("Percentage").toInt ();
		info.TimeToFull_ = face.property ("TimeToFull").toLongLong ();
		info.TimeToEmpty_ = face.property ("TimeToEmpty").toLongLong ();
		info.Voltage_ = face.property ("Voltage").toDouble ();
		info.Energy_ = face.property ("Energy").toDouble ();
		info.EnergyFull_ = face.property ("EnergyFull").toDouble ();
		info.DesignEnergyFull_ = face.property ("EnergyFullDesign").toDouble ();
		info.EnergyRate_ = face.property ("EnergyRate").toDouble ();
		info.Technology_ = TechIdToString (face.property ("Technology").toInt ());
		info.Temperature_ = 0;

		emit batteryInfoUpdated (info);

		if (!HasGlobalDeviceChanged_ && !SubscribedDevices_.contains (id))
		{
			SB_.connect ("org.freedesktop.UPower",
					id,
					"org.freedesktop.DBus.Properties",
					"PropertiesChanged",
					this,
					SLOT (handlePropertiesChanged (QDBusMessage)));
			SubscribedDevices_ << id;
		}
	}

	void UPowerConnector::handlePropertiesChanged (const QDBusMessage& msg)
	{
		const auto& changedVar = msg.arguments ().value (1);
		const auto& changedMap = qdbus_cast<QVariantMap> (changedVar.value<QDBusArgument> ());
		if (!changedMap.contains ("Percentage"))
			return;

		requeryDevice (msg.path ());
	}
}
}
}
