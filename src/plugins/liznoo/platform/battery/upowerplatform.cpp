/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upowerplatform.h"
#include "../common/dbusthread.h"
#include "../upower/upowerconnector.h"

namespace LC
{
namespace Liznoo
{
namespace Battery
{
	UPowerPlatform::UPowerPlatform (const UPower::UPowerThread_ptr& thread, QObject *parent)
	: BatteryPlatform { parent }
	, Thread_ { thread }
	{
		Thread_->ScheduleImpl ([this] (UPower::UPowerConnector *conn)
				{
					connect (conn,
							SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)),
							this,
							SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)));
					QMetaObject::invokeMethod (conn,
							"enumerateDevices",
							Qt::QueuedConnection);
				});
	}
}
}
}
