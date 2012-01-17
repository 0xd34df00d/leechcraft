//------------------------------------------------------------------------------------------------------------------------------------------
//    Copyright (c) 2011, Eugene Mamin <TheDZhon@gmail.com>
//    All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//        * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//        * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//        * Neither the name of the Prefix Increment nor the
//        names of its contributors may be used to endorse or promote products
//        derived from this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY Eugene Mamin <TheDZhon@gmail.com> ''AS IS'' AND ANY
//    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL Eugene Mamin <TheDZhon@gmail.com> BE LIABLE FOR ANY
//    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------------------------------------------------------------------

#include <QtGlobal>
#include "fakeqwidgetwinapi.h"
#include "platformwinapi.h"

namespace LeechCraft
{
namespace Liznoo
{
	PlatformWinAPI::PlatformWinAPI (QObject* parent)
	: PlatformLayer(parent)
	, HPowerSchemeNotify_(new HPOWERNOTIFY)
	, HPowerSourceNotify_(new HPOWERNOTIFY)
	, HBatteryPowerNotify_(new HPOWERNOTIFY)
	, FakeWidget_(new FakeQWidgetWinAPI)
	{
		FakeWidget_->hide();

		HWND h_wnd = FakeWidget_->winId();

		*HPowerSchemeNotify_ = RegisterPowerSettingNotification (
				h_wnd, &GUID_POWERSCHEME_PERSONALITY,
				DEVICE_NOTIFY_WINDOW_HANDLE);

		Q_ASSERT(*HPowerSchemeNotify_);

		*HPowerSourceNotify_ = RegisterPowerSettingNotification (
			h_wnd, &GUID_ACDC_POWER_SOURCE,
			DEVICE_NOTIFY_WINDOW_HANDLE);

		Q_ASSERT(*HPowerSourceNotify_);

		*HBatteryPowerNotify_ = RegisterPowerSettingNotification (
			h_wnd, &GUID_BATTERY_PERCENTAGE_REMAINING,
			DEVICE_NOTIFY_WINDOW_HANDLE );		

		Q_ASSERT(*HBatteryPowerNotify_);

		connect (FakeWidget_.get(), SIGNAL(schemeChanged(QString)), 
			this, SLOT(handleSchemeChanged(QString)));
		connect (FakeWidget_.get(), SIGNAL(powerSourceChanged(QString)), 
			this, SLOT(handleSchemeChanged(QString)));
		connect (FakeWidget_.get(), SIGNAL(batteryStateChanged(int)), 
			this, SLOT(handleSchemeChanged(QString)));

		emit started();
	}

	void PlatformWinAPI::Stop()
	{
		HPowerSchemeNotify_.reset(nullptr);
		HPowerSourceNotify_.reset(nullptr);
		HBatteryPowerNotify_.reset(nullptr);
		FakeWidget_.reset(nullptr);
	}

	void PlatformWinAPI::handleSchemeChanged( QString schemeName )
	{
		qDebug() << tr("New power scheme detected") << ": [" << schemeName << "]";
	}

	void PlatformWinAPI::handlePowerSourceChanged( QString powerSource )
	{
		qDebug() << tr("New power source detected") << ": [" << powerSource << "]";
	}

	void PlatformWinAPI::handleBatteryStateChanged( int newPercentage )
	{
		qDebug() << tr("New battery state detected") << ": [" << newPercentage << "]";

		BatteryInfo info;
		info.Percentage_ = newPercentage;

		emit batteryInfoUpdated(info);
	}

} // namespace Liznoo
} // namespace LeechCraft