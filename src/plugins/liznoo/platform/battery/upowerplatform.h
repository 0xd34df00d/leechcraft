/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "batteryplatform.h"

namespace LC
{
namespace Liznoo
{
template<typename>
class DBusThread;

namespace UPower
{
	class UPowerConnector;

	using UPowerThread_ptr = std::shared_ptr<DBusThread<UPowerConnector>>;
}

namespace Battery
{
	class UPowerPlatform : public BatteryPlatform
	{
		const UPower::UPowerThread_ptr Thread_;
	public:
		UPowerPlatform (const UPower::UPowerThread_ptr&, QObject* = nullptr);
	};
}
}
}
