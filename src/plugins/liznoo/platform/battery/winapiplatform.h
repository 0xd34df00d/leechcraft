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
namespace WinAPI
{
	class FakeQWidgetWinAPI;
	using FakeQWidgetWinAPI_ptr = std::shared_ptr<FakeQWidgetWinAPI>;
}

namespace Battery
{
	class WinAPIPlatform : public BatteryPlatform
	{
		Q_OBJECT

		const WinAPI::FakeQWidgetWinAPI_ptr Widget_;
	public:
		WinAPIPlatform (const WinAPI::FakeQWidgetWinAPI_ptr&, QObject* = nullptr);
	private slots:
		void handleBatteryStateChanged (int newPercentage);
	};
}
}
}
