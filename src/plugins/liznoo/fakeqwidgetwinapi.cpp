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

#include "fakeqwidgetwinapi.h"

namespace LeechCraft 
{
namespace Liznoo
{
	FakeQWidgetWinAPI::FakeQWidgetWinAPI (QWidget *parent)
	: QWidget (parent)
	{

	}

	void FakeQWidgetWinAPI::prepareSchemeChange (PPOWERBROADCAST_SETTING setting)
	{
		const GUID newScheme = 
			*reinterpret_cast<GUID*> (reinterpret_cast<DWORD_PTR> (setting->Data));

		QString scheme;
		if (GUID_MAX_POWER_SAVINGS == newScheme)
			scheme = tr ("Power saver");
		else if (GUID_MIN_POWER_SAVINGS == newScheme)
			scheme = tr ("High performance");
		else 
			scheme = tr ("Balanced");

		emit schemeChanged (scheme);
	}

	void FakeQWidgetWinAPI::preparePowerSourceChange (PPOWERBROADCAST_SETTING setting)
	{
		const int nPowerSrc = 
			*reinterpret_cast<int*> (reinterpret_cast<DWORD_PTR> (setting->Data));

		const QString& powerSource = nPowerSrc ? tr ("Battery") : tr ("AC");

		emit powerSourceChanged (powerSource);
	}

	void FakeQWidgetWinAPI::prepareBatteryStateChange (PPOWERBROADCAST_SETTING setting)
	{
		const int nPercentLeft = 
			*reinterpret_cast<int*> (reinterpret_cast<DWORD_PTR> (setting->Data));

		emit batteryStateChanged (nPercentLeft);
	}

	bool FakeQWidgetWinAPI::winEvent (MSG *message, long *result)
	{
		if(message->message == WM_POWERBROADCAST)
		{	
			Q_ASSERT (message->wParam == PBT_POWERSETTINGCHANGE);

			const PPOWERBROADCAST_SETTING rcvd_setting = 
				reinterpret_cast<PPOWERBROADCAST_SETTING> (message->lParam);

			if (sizeof (GUID) == rcvd_setting->DataLength &&
				rcvd_setting->PowerSetting == GUID_POWERSCHEME_PERSONALITY)
				prepareSchemeChange (rcvd_setting);
			else if (sizeof (int) == rcvd_setting->DataLength &&
				rcvd_setting->PowerSetting == GUID_ACDC_POWER_SOURCE)
				preparePowerSourceChange (rcvd_setting);
			else if (sizeof (int) == rcvd_setting->DataLength &&
				rcvd_setting->PowerSetting == GUID_BATTERY_PERCENTAGE_REMAINING)
				prepareBatteryStateChange (rcvd_setting);
		}
		return QWidget::winEvent (message, result);
	}	
} // namespace Liznoo
} // namespace Leechcraft