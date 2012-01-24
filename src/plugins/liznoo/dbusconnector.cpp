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

#include "dbusconnector.h"
#include <QTimer>
#include <QDBusInterface>
#include <QDBusReply>
#include <QtDebug>
#include <util/util.h>
#include "batteryinfo.h"

namespace LeechCraft
{
namespace Liznoo
{
	DBusConnector::DBusConnector (QObject *parent)
	: QObject (parent)
	, SB_ (QDBusConnection::systemBus ())
	{
		SB_.connect ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				"DeviceAdded",
				this,
				SLOT (requeryDevice (const QString&)));
		SB_.connect ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				"DeviceChanged",
				this,
				SLOT (requeryDevice (const QString&)));
		SB_.connect ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				"Sleeping",
				this,
				SLOT (handleGonnaSleep ()));
		SB_.connect ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				"Resuming",
				this,
				SLOT (handleWokeUp ()));

		QTimer::singleShot (1000,
				this,
				SLOT (enumerateDevices ()));
	}

	void DBusConnector::changeState (PlatformLayer::PowerState state)
	{
		QDBusInterface face ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				SB_);

		auto st2meth = [] (PlatformLayer::PowerState state)
		{
			switch (state)
			{
			case PlatformLayer::PowerState::Suspend:
				return "Suspend";
			case PlatformLayer::PowerState::Hibernate:
				return "Hibernate";
			}
		};

		face.call (QDBus::NoBlock, st2meth (state));
	}

	void DBusConnector::handleGonnaSleep ()
	{
		Entity e = Util::MakeEntity ("Sleeping",
				QString (),
				TaskParameter::Internal,
				"x-leechcraft/power-state-changed");
		e.Additional_ ["TimeLeft"] = 1000;
		emit gotEntity (e);
	}

	void DBusConnector::handleWokeUp ()
	{
		Entity e = Util::MakeEntity ("WokeUp",
				QString (),
				TaskParameter::Internal,
				"x-leechcraft/power-state-changed");
		emit gotEntity (e);
	}

	void DBusConnector::enumerateDevices()
	{
		QDBusInterface face ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				SB_);

		auto res = face.call ("EnumerateDevices");
		Q_FOREACH (QVariant argument, res.arguments ())
		{
			auto arg = argument.value<QDBusArgument> ();
			QStringList paths;
			arg >> paths;
			Q_FOREACH (const QString& path, paths)
				requeryDevice (path);
		}
	}

	namespace
	{
		QString TechIdToString (int id)
		{
			QMap<int, QString> id2str;
			id2str [1] = "Li-Ion";
			id2str [2] = "Li-Polymer";
			id2str [3] = "Li-Iron-Phosphate";
			id2str [4] = "Lead acid";
			id2str [5] = "NiCd";
			id2str [6] = "NiMh";

			return id2str.value (id, "<unknown>");
		}
	}

	void DBusConnector::requeryDevice (const QString& id)
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
		info.EnergyRate_ = face.property ("EnergyRate").toDouble ();
		info.Technology_ = TechIdToString (face.property ("Technology").toInt ());

		emit batteryInfoUpdated (info);
	}
}
}
