/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upowerplatform.h"
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>
#include "../dbus/endpoints.h"

namespace LC::Liznoo::Battery
{
	UPowerPlatform::UPowerPlatform (QObject *parent)
	: BatteryPlatform { parent }
	, UPower_ { DBus::GetUPowerEndpoint () }
	{
		UPower_.Connect ("DeviceAdded", [this] (const QDBusObjectPath& path) { RequeryDevice (path.path ()); });
		EnumerateDevices ();
	}

	Util::ContextTask<void> UPowerPlatform::EnumerateDevices ()
	{
		co_await Util::AddContextObject { *this };
		const auto& eitherPaths = co_await UPower_.Call<QList<QDBusObjectPath>> ("EnumerateDevices"_qs);
		const auto& paths = co_await Util::WithHandler (eitherPaths,
				[] (const QDBusError& err) { qWarning () << "unable to enumerate devices:" << err; });
		for (const auto& path : paths)
			RequeryDevice (path.path ());
	}

	namespace
	{
		QString TechIdToString (int id)
		{
			switch (id)
			{
			case 1:
				return "Li-Ion"_qs;
			case 2:
				return "Li-Polymer"_qs;
			case 3:
				return "Li-Iron-Phosphate"_qs;
			case 4:
				return "Lead acid"_qs;
			case 5:
				return "NiCd"_qs;
			case 6:
				return "NiMh"_qs;
			default:
				qWarning () << "unknown technology ID" << id;
				return "<unknown>"_qs;
			}
		}
	}

	Util::ContextTask<void> UPowerPlatform::RequeryDevice (QString id)
	{
		co_await Util::AddContextObject { *this };
		const Util::DBus::Endpoint device
		{
			.Service = "org.freedesktop.UPower"_qs,
			.Path = id,
			.Interface = "org.freedesktop.UPower.Device"_qs,
			.Conn = QDBusConnection::systemBus (),
		};
		const auto& eitherProps = co_await device.GetAllProperties ();
		const auto& props = co_await Util::WithHandler (eitherProps,
				[&] (const QDBusError& err) { qWarning () << "unable to query props on" << id << err; });
		if (props ["Type"_qs].toInt () != 2)
			co_return;

		BatteryInfo info;
		info.ID_ = id;
		info.Percentage_ = props ["Percentage"_qs].toInt ();
		info.TimeToFull_ = props ["TimeToFull"_qs].toLongLong ();
		info.TimeToEmpty_ = props ["TimeToEmpty"_qs].toLongLong ();
		info.Voltage_ = props ["Voltage"_qs].toDouble ();
		info.Energy_ = props ["Energy"_qs].toDouble ();
		info.EnergyFull_ = props ["EnergyFull"_qs].toDouble ();
		info.DesignEnergyFull_ = props ["EnergyFullDesign"_qs].toDouble ();
		info.EnergyRate_ = props ["EnergyRate"_qs].toDouble ();
		info.Technology_ = TechIdToString (props ["Technology"_qs].toInt ());
		info.Temperature_ = 0;

		emit batteryInfoUpdated (info);

		if (!SubscribedDevices_.contains (id))
		{
			QDBusConnection::systemBus ().connect ("org.freedesktop.UPower"_qs,
					id,
					"org.freedesktop.DBus.Properties"_qs,
					"PropertiesChanged"_qs,
					this,
					SLOT (handlePropertiesChanged (QDBusMessage)));
			SubscribedDevices_ << id;
		}
	}

	void UPowerPlatform::handlePropertiesChanged (const QDBusMessage& msg)
	{
		const auto& changedVar = msg.arguments ().value (1);
		const auto& changedMap = qdbus_cast<QVariantMap> (changedVar.value<QDBusArgument> ());
		if (!changedMap.contains ("Percentage"_qs))
			return;

		RequeryDevice (msg.path ());
	}
}
