/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "batteryhistory.h"
#include "batteryinfo.h"

namespace LC
{
namespace Liznoo
{
	BatteryHistory::BatteryHistory (const BatteryInfo& info)
	: Percentage_ (info.Percentage_)
	, Voltage_ (info.Voltage_)
	, Energy_ (info.Energy_)
	, EnergyRate_ (info.EnergyRate_)
	, Temperature_ (info.Temperature_)
	{
	}
}
}
