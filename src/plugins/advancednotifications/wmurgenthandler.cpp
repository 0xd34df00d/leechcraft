/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "wmurgenthandler.h"
#include <QMainWindow>
#include <QApplication>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "core.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	WMUrgentHandler::WMUrgentHandler ()
	{
	}

	NotificationMethod WMUrgentHandler::GetHandlerMethod () const
	{
		return NMUrgentHint;
	}

	void WMUrgentHandler::Handle (const Entity& e, const NotificationRule&)
	{
		if (e.Additional_ ["org.LC.AdvNotifications.EventCategory"].toString () == "org.LC.AdvNotifications.Cancel")
			return;

		auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		auto win = rootWM->GetPreferredWindow ();

		if (!win->isActiveWindow ())
			QApplication::alert (win);
	}
}
}
