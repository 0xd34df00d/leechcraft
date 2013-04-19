/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012       Eugene Mamin
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include <QtGlobal>
#include <QTimer>
#include "fakeqwidgetwinapi.h"
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

namespace LeechCraft
{
namespace Liznoo
{
	PlatformWinAPI::PlatformWinAPI (QObject* parent)
	: PlatformLayer (parent)
	, HPowerSchemeNotify_ (new HPOWERNOTIFY, aHPowerNotifyDeleter)
	, HPowerSourceNotify_ (new HPOWERNOTIFY, aHPowerNotifyDeleter)
	, HBatteryPowerNotify_ (new HPOWERNOTIFY, aHPowerNotifyDeleter)
	, FakeWidget_ (new FakeQWidgetWinAPI)
	{
		FakeWidget_->hide ();

		HWND h_wnd = FakeWidget_->winId ();

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
		connect (FakeWidget_.get (), 
				SIGNAL (batteryStateChanged (int)), 
				this, 
				SLOT (handleBatteryStateChanged (int)));

		QTimer::singleShot (0, 
				this, 
				SIGNAL (started ()));
	}

	void PlatformWinAPI::Stop ()
	{
		HPowerSchemeNotify_.reset (nullptr);
		HPowerSourceNotify_.reset (nullptr);
		HBatteryPowerNotify_.reset (nullptr);
		FakeWidget_.reset (nullptr);
	}

	void PlatformWinAPI::handleSchemeChanged (QString schemeName)
	{
		qDebug() << "New power scheme detected" << ": [" << schemeName << "]";
	}

	void PlatformWinAPI::handlePowerSourceChanged (QString powerSource)
	{
		qDebug() << "New power source detected" << ": [" << powerSource << "]";
	}

	void PlatformWinAPI::handleBatteryStateChanged (int newPercentage)
	{
		//TODO(DZhon): Rewrite using Win32_Battery WMI Class.

		qDebug() << "New battery state detected" << ": [" << newPercentage << "]";

		SYSTEM_POWER_STATUS powerStatus;
		BOOL retCode = GetSystemPowerStatus (&powerStatus);

		Q_ASSERT (retCode);

		BatteryInfo info;		

		info.TimeToEmpty_ = powerStatus.BatteryLifeTime;
		info.Percentage_ = newPercentage;

		emit batteryInfoUpdated (info);
	}
} // namespace Liznoo
} // namespace LeechCraft
