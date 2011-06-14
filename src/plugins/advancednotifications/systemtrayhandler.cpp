/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "systemtrayhandler.h"
#include <interfaces/structures.h>
#include <QSystemTrayIcon>
#include "generalhandler.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	SystemTrayHandler::SystemTrayHandler ()
	{
	}

	ConcreteHandlerBase::HandlerType SystemTrayHandler::GetHandlerType () const
	{
		return HTSystemTray;
	}

	void SystemTrayHandler::Handle (const Entity& e)
	{
		const QString& cat = e.Additional_ ["org.LC.AdvNotifications.EventCategory"].toString ();
		
		PrepareSysTrayIcon (cat);
	}
	
	void SystemTrayHandler::PrepareSysTrayIcon (const QString& category)
	{
		if (Category2Icon_.contains (category))
			return;
		
		Category2Icon_ [category] = new QSystemTrayIcon (GH_->GetIconForCategory (category));
		Category2Icon_ [category]->show ();
	}
}
}
