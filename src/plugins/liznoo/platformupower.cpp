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

#include "platformupower.h"
#include "dbusconnector.h"
#include "dbusthread.h"

namespace LeechCraft
{
namespace Liznoo
{
	PlatformUPower::PlatformUPower (QObject *parent)
	: PlatformLayer (parent)
	{
		qRegisterMetaType<PlatformLayer::PowerState> ("Liznoo::PlatformLayer::PowerState");

		Thread_ = new DBusThread;
		connect (Thread_,
				SIGNAL(started ()),
				this,
				SLOT (handleThreadStarted ()));
		Thread_->start (QThread::LowestPriority);
	}

	void PlatformUPower::Stop ()
	{
		if (!Thread_->wait (1000))
			Thread_->terminate ();
	}

	void PlatformUPower::ChangeState (PowerState state)
	{
		QMetaObject::invokeMethod (Thread_->GetConnector (),
				"changeState",
				Qt::QueuedConnection,
				Q_ARG (Liznoo::PlatformLayer::PowerState, state));
	}

	void PlatformUPower::handleThreadStarted ()
	{
		emit started ();

		connect (Thread_->GetConnector (),
				SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)),
				this,
				SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)));
		connect (Thread_->GetConnector (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}
}
}
