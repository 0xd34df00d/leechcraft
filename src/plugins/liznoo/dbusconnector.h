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

#pragma once

#include <QObject>
#include <QDBusConnection>
#include <interfaces/structures.h>
#include "batteryinfo.h"
#include "platformlayer.h"

namespace LeechCraft
{
namespace Liznoo
{
	class DBusConnector : public QObject
	{
		Q_OBJECT

		QDBusConnection SB_;
	public:
		DBusConnector (QObject* = 0);
	public slots:
		void changeState (Liznoo::PlatformLayer::PowerState);
	private slots:
		void handleGonnaSleep ();
		void handleWokeUp ();
		void enumerateDevices ();
		void requeryDevice (const QString&);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void batteryInfoUpdated (Liznoo::BatteryInfo);
	};
}
}

