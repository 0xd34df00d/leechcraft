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

#include "handlersconfigurator.h"
#include <interfaces/structures.h>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	HandlersConfigurator::HandlersConfigurator (QObject *parent)
	: QObject (parent)
	{
	}
	
	QSet<ConcreteHandlerBase::HandlerType> HandlersConfigurator::GetEnabledHandlers (const Entity& e) const
	{
		QSet<ConcreteHandlerBase::HandlerType> result;

		if (e.Additional_ ["org.LC.AdvNotifications.EventCategory"] == "org.LC.AdvNotifications.Cancel")
		{
			result << ConcreteHandlerBase::HTSystemTray;
			result << ConcreteHandlerBase::HTLCTray;
		}

		if (e.Additional_ ["org.LC.AdvNotifications.EventCategory"] == "org.LC.AdvNotifications.IM" &&
				e.Additional_ ["org.LC.AdvNotifications.EventType"] != "org.LC.AdvNotifications.IM.MUCMessage")
		{
			result << ConcreteHandlerBase::HTSystemTray;
			result << ConcreteHandlerBase::HTAudioNotification;
		}
		
		return result;
	}
}
}
