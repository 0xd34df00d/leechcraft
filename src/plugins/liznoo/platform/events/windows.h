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
#include "platform.h"

namespace LC::Liznoo::Windows
{
    class FakeQWidgetWinAPI;
    using FakeQWidgetWinAPI_ptr = std::shared_ptr<FakeQWidgetWinAPI>;
}

namespace LC::Liznoo::Events
{
	class Windows : public Platform
	{
		Q_OBJECT

		typedef std::function<void (HPOWERNOTIFY*)> HPowerNotifyDeleter;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSchemeNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSourceNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HBatteryPowerNotify_;

		const Windows::FakeQWidgetWinAPI_ptr FakeWidget_;
	public:
		Windows (const Windows::FakeQWidgetWinAPI_ptr&, QObject* = 0);
	private slots:
		void handleSchemeChanged (QString schemeName);
		void handlePowerSourceChanged (QString powerSource);
	};
}
