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

#include "fakeqwidgetwinapi.h"

namespace LeechCraft 
{
namespace Liznoo
{
	FakeQWidgetWinAPI::FakeQWidgetWinAPI (QWidget * parent)
	: QWidget(parent)
	{

	}

	void FakeQWidgetWinAPI::prepareSchemeChange(PPOWERBROADCAST_SETTING setting )
	{
		GUID newScheme = *(GUID*)(DWORD_PTR)setting->Data;

		QString scheme;
		if ( GUID_MAX_POWER_SAVINGS == newScheme )
		{
			scheme = tr("Power saver");
		}
		else if ( GUID_MIN_POWER_SAVINGS == newScheme )
		{
			scheme = tr("High performance");
		}
		else 
		{
			scheme = tr("Balanced");
		}

		emit schemeChanged(scheme);
	}

	void FakeQWidgetWinAPI::preparePowerSourceChange(PPOWERBROADCAST_SETTING setting )
	{
		int nPowerSrc = *(int*)(DWORD_PTR) setting->Data;
		QString powerSource = (0 != nPowerSrc) ? tr("Battery") : tr("AC");

		emit powerSourceChanged(powerSource);
	}

	void FakeQWidgetWinAPI::prepareBatteryStateChange(PPOWERBROADCAST_SETTING setting )
	{
		int nPercentLeft = *(int*)(DWORD_PTR) setting->Data;

		emit batteryStateChanged(nPercentLeft);
	}

	bool FakeQWidgetWinAPI::winEvent( MSG * message, long * result )
	{
		if(message->message == WM_POWERBROADCAST)
		{	
			Q_ASSERT(message->wParam == PBT_POWERSETTINGCHANGE);

			PPOWERBROADCAST_SETTING rcvd_setting = 
				reinterpret_cast<PPOWERBROADCAST_SETTING>(message->lParam);

			if (sizeof(GUID) == rcvd_setting->DataLength &&
				rcvd_setting->PowerSetting == GUID_POWERSCHEME_PERSONALITY )
			{
				prepareSchemeChange(rcvd_setting);
			} 
			else if (sizeof(int) == rcvd_setting->DataLength &&
				rcvd_setting->PowerSetting == GUID_ACDC_POWER_SOURCE)
			{
				preparePowerSourceChange(rcvd_setting);
			} 
			else if (sizeof(int) == rcvd_setting->DataLength &&
				rcvd_setting->PowerSetting == GUID_BATTERY_PERCENTAGE_REMAINING )
			{
				prepareBatteryStateChange(rcvd_setting);
			}
		}
		return QWidget::winEvent(message, result);
	}	
} // namespace Liznoo
} // namespace Leechcraft