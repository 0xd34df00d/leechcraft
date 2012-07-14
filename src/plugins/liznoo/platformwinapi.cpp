/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012       Eugene Mamin
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
		qDebug() << tr ("New power scheme detected") << ": [" << schemeName << "]";
	}

	void PlatformWinAPI::handlePowerSourceChanged (QString powerSource)
	{
		qDebug() << tr ("New power source detected") << ": [" << powerSource << "]";
	}

	void PlatformWinAPI::handleBatteryStateChanged (int newPercentage)
	{
		//TODO(DZhon): Rewrite using Win32_Battery WMI Class.

		qDebug() << tr ("New battery state detected") << ": [" << newPercentage << "]";

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