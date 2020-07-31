/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012       Eugene Mamin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fakeqwidgetwinapi.h"
#include <QDebug>
#include <objbase.h>

namespace LC
{
namespace Liznoo
{
namespace WinAPI
{
	FakeQWidgetWinAPI::FakeQWidgetWinAPI (QWidget *parent)
	: QWidget (parent)
	{
		hide ();
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

	void FakeQWidgetWinAPI::powerSettingsChanged (PPOWERBROADCAST_SETTING setting)
	{
		if (!setting)
		{
			qWarning()
					<< Q_FUNC_INFO
					<< "Null setting received";
			return;
		}
		if (sizeof (GUID) == setting->DataLength &&
			IsEqualGUID (setting->PowerSetting, GUID_POWERSCHEME_PERSONALITY))
			prepareSchemeChange (setting);
		else if (sizeof (int) == setting->DataLength &&
			IsEqualGUID (setting->PowerSetting, GUID_ACDC_POWER_SOURCE))
			preparePowerSourceChange (setting);
		else if (sizeof (int) == setting->DataLength &&
			IsEqualGUID (setting->PowerSetting, GUID_BATTERY_PERCENTAGE_REMAINING))
			prepareBatteryStateChange (setting);
	}

	bool FakeQWidgetWinAPI::nativeEvent (const QByteArray &eventType, void *msg, long *result)
	{
		static const QByteArray kGenericMSG { "windows_generic_MSG" };
		if (eventType != kGenericMSG)
			return QWidget::nativeEvent (eventType, msg, result);

		MSG *const message = reinterpret_cast<MSG *> (msg);

		if (message->message == WM_POWERBROADCAST && message->wParam == PBT_POWERSETTINGCHANGE)
			powerSettingsChanged (reinterpret_cast<PPOWERBROADCAST_SETTING> (message->lParam));

		return QWidget::nativeEvent (eventType, msg, result);
	}
}
}
}
