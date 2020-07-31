/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "batteryinfo.h"
#include <QtDebug>

namespace LC
{
namespace Liznoo
{
	void BatteryInfo::Dump ()
	{
		qDebug () << Q_FUNC_INFO
				<< ID_
				<< Percentage_
				<< TimeToFull_
				<< TimeToEmpty_
				<< Voltage_
				<< Energy_
				<< EnergyFull_
				<< EnergyRate_
				<< Technology_
				<< Temperature_
				<< CyclesCount_;
	}
}
}
