/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "winapiplatform.h"
#include <QtDebug>
#include "../winapi/fakeqwidgetwinapi.h"

namespace LC
{
namespace Liznoo
{
namespace Battery
{
	WinAPIPlatform::WinAPIPlatform (const WinAPI::FakeQWidgetWinAPI_ptr& widget, QObject *parent)
	: BatteryPlatform { parent }
	, Widget_ { widget }
	{
		connect (Widget_.get (),
				SIGNAL (batteryStateChanged (int)),
				this,
				SLOT (handleBatteryStateChanged (int)));
	}

	void WinAPIPlatform::handleBatteryStateChanged (int newPercentage)
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
}
}
