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

#include <QWidget>
#include <QString>
#include <windows.h>

namespace LeechCraft 
{
namespace Liznoo
{
	class FakeQWidgetWinAPI : public QWidget
	{
		Q_OBJECT
	public:
		FakeQWidgetWinAPI (QWidget *parent = NULL);
	protected:
		virtual void prepareSchemeChange (PPOWERBROADCAST_SETTING setting);
		virtual void preparePowerSourceChange (PPOWERBROADCAST_SETTING setting);
		virtual void prepareBatteryStateChange (PPOWERBROADCAST_SETTING setting);

		virtual bool winEvent (MSG *message, long *result);
	signals:
		void schemeChanged (QString schemeName);
		void powerSourceChanged (QString powerSource);
		void batteryStateChanged (int newPercentage);
	};
} // namespace Liznoo
} // namespace Leechcraft

