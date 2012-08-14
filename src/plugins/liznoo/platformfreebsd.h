/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
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

#include "platformlayer.h"
#include <QTimer>

namespace LeechCraft
{
namespace Liznoo
{
	class PlatformFreeBSD : public PlatformLayer
	{
		Q_OBJECT

	public:
		PlatformFreeBSD (QObject* = 0);
		void Stop ();
		void ChangeState (PowerState);
	private slots:
		void update();
	private:
		QTimer *timer;
		static const int updateInterval = 10*1000;
		int acpifd;
	signals:
		void batteryInfoUpdated (Liznoo::BatteryInfo);
	};
}
}
