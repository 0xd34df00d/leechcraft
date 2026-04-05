/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "windows.h"
#include <QtDebug>
#include "../winapi/fakeqwidgetwinapi.h"

namespace LC::Liznoo::Battery
{
	Windows::Windows (const Liznoo::Windows::FakeQWidgetWinAPI_ptr& widget, QObject *parent)
	: Platform { parent }
	, Widget_ { widget }
	{
		connect (Widget_.get (),
				SIGNAL (batteryStateChanged (int)),
				this,
				SLOT (handleBatteryStateChanged (int)));
	}

	void Windows::handleBatteryStateChanged (int newPercentage)
	{
		//TODO(DZhon): Rewrite using Win32_Battery WMI Class.

		qDebug () << "New battery state detected" << ": [" << newPercentage << "]";

		SYSTEM_POWER_STATUS powerStatus;
		BOOL retCode = GetSystemPowerStatus (&powerStatus);

		Q_ASSERT (retCode);

		BatteryInfo info;

		info.TimeToEmpty_ = std::max<DWORD> (powerStatus.BatteryLifeTime, 0);
		info.Percentage_ = newPercentage;

		emit batteryInfoUpdated (info);
	}
}
