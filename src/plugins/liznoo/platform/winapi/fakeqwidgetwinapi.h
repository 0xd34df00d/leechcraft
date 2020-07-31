/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012       Eugene Mamin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QString>
#include <windows.h>

namespace LC
{
namespace Liznoo
{
namespace WinAPI
{
	class FakeQWidgetWinAPI : public QWidget
	{
		Q_OBJECT
	public:
		FakeQWidgetWinAPI (QWidget *parent = NULL);
	private:
		void prepareSchemeChange (PPOWERBROADCAST_SETTING setting);
		void preparePowerSourceChange (PPOWERBROADCAST_SETTING setting);
		void prepareBatteryStateChange (PPOWERBROADCAST_SETTING setting);

		void powerSettingsChanged (PPOWERBROADCAST_SETTING setting);

		bool nativeEvent (const QByteArray &eventType, void *message, long *result) override;
	signals:
		void schemeChanged (QString schemeName);
		void powerSourceChanged (QString powerSource);
		void batteryStateChanged (int newPercentage);
	};
}
}
}
