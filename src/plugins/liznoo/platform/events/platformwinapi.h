/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012       Eugene Mamin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <windows.h>
#include "platformlayer.h"

namespace LC
{
namespace Liznoo
{
namespace WinAPI
{
    class FakeQWidgetWinAPI;
    using FakeQWidgetWinAPI_ptr = std::shared_ptr<FakeQWidgetWinAPI>;
}

namespace Events
{
	class PlatformWinAPI : public PlatformLayer
	{
		Q_OBJECT

		typedef std::function<void (HPOWERNOTIFY*)> HPowerNotifyDeleter;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSchemeNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSourceNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HBatteryPowerNotify_;

		const WinAPI::FakeQWidgetWinAPI_ptr FakeWidget_;
	public:
		PlatformWinAPI (const WinAPI::FakeQWidgetWinAPI_ptr&, const ICoreProxy_ptr&, QObject* = 0);
	private slots:
		void handleSchemeChanged (QString schemeName);
		void handlePowerSourceChanged (QString powerSource);
	};
}
} // namespace Liznoo
} // namespace Leechcraft
