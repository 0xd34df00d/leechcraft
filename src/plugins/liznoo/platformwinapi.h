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

#pragma once

#include <memory>
#include <functional>
#include <windows.h>
#include "platformlayer.h"

namespace LeechCraft 
{
namespace Liznoo
{
	class FakeQWidgetWinAPI;
	class PlatformWinAPI : public PlatformLayer
	{
		Q_OBJECT

		typedef std::function<void (HPOWERNOTIFY*)> HPowerNotifyDeleter;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSchemeNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSourceNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HBatteryPowerNotify_;
		std::unique_ptr<FakeQWidgetWinAPI> FakeWidget_;
	public:
		PlatformWinAPI (QObject* = 0);
		virtual void Stop ();
	private slots:
		void handleSchemeChanged(QString schemeName);
		void handlePowerSourceChanged(QString powerSource);
		void handleBatteryStateChanged(int newPercentage);
	};
} // namespace Liznoo
} // namespace Leechcraft
