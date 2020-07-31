/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012       Eugene Mamin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QtGlobal>
#include <QTimer>
#include "../winapi/fakeqwidgetwinapi.h"
#include "platformwinapi.h"

namespace
{
auto aHPowerNotifyDeleter = [] (HPOWERNOTIFY *ptr)
{
	if (ptr && *ptr)
	{
		UnregisterPowerSettingNotification (*ptr);
		delete ptr;
	}
};
}

namespace LC
{
namespace Liznoo
{
namespace Events
{
	PlatformWinAPI::PlatformWinAPI (const WinAPI::FakeQWidgetWinAPI_ptr& widget, const ICoreProxy_ptr& proxy, QObject* parent)
	: PlatformLayer (proxy, parent)
	, HPowerSchemeNotify_ (new HPOWERNOTIFY, aHPowerNotifyDeleter)
	, HPowerSourceNotify_ (new HPOWERNOTIFY, aHPowerNotifyDeleter)
	, HBatteryPowerNotify_ (new HPOWERNOTIFY, aHPowerNotifyDeleter)
	, FakeWidget_ (widget)
	{
		HWND h_wnd = reinterpret_cast<HWND> (FakeWidget_->winId ());

		*HPowerSchemeNotify_ = RegisterPowerSettingNotification (h_wnd,
			&GUID_POWERSCHEME_PERSONALITY, DEVICE_NOTIFY_WINDOW_HANDLE);

		Q_ASSERT (*HPowerSchemeNotify_);

		*HPowerSourceNotify_ = RegisterPowerSettingNotification (h_wnd,
			&GUID_ACDC_POWER_SOURCE, DEVICE_NOTIFY_WINDOW_HANDLE);

		Q_ASSERT (*HPowerSourceNotify_);

		*HBatteryPowerNotify_ = RegisterPowerSettingNotification (h_wnd,
			&GUID_BATTERY_PERCENTAGE_REMAINING,	DEVICE_NOTIFY_WINDOW_HANDLE);

		Q_ASSERT (*HBatteryPowerNotify_);

		connect (FakeWidget_.get (),
				SIGNAL (schemeChanged (QString)),
				this,
				SLOT (handleSchemeChanged (QString)));
		connect (FakeWidget_.get (),
				SIGNAL (powerSourceChanged (QString)),
				this,
				SLOT (handlePowerSourceChanged (QString)));
	}

	void PlatformWinAPI::handleSchemeChanged (QString schemeName)
	{
		qDebug() << "New power scheme detected" << ": [" << schemeName << "]";
	}

	void PlatformWinAPI::handlePowerSourceChanged (QString powerSource)
	{
		qDebug() << "New power source detected" << ": [" << powerSource << "]";
	}
}
} // namespace Liznoo
} // namespace LC
