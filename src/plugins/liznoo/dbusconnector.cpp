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

#include "dbusconnector.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QtDebug>
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
		
		QDBusInterface face ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				SB_);
		
		auto res = face.call ("EnumerateDevices");
		qDebug () << "devices:" << res.arguments ();
		Q_FOREACH (QVariant argument, res.arguments ())
			qDebug () << argument.toString ();
	}
	
	void DBusConnector::requeryDevice (const QString& id)
	{
		QDBusInterface face ("org.freedesktop.UPower",
				id,
				"org.freedesktop.UPower.Device",
				SB_);
		auto res = face.call ("Type");
		qDebug () << Q_FUNC_INFO << res.errorMessage ();
		qDebug () << face.property ("Type");
		if (QDBusReply<int> (face.call ("Type")) != 2)
			return;
		
		BatteryInfo info;
		info.ID_ = id;
		info.Percentage_ = QDBusReply<double> (face.call ("Percentage"));
		info.TimeToFull_ = QDBusReply<qlonglong> (face.call ("TimeToFull"));
		info.TimeToEmpty_ = QDBusReply<qlonglong> (face.call ("TimeToEmpty"));
		
		info.Dump ();
	}
}
}
