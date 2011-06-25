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

#include "generalhandler.h"
#include <interfaces/structures.h>
#include "systemtrayhandler.h"
#include "handlersconfigurator.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	GeneralHandler::GeneralHandler (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	, HandlersConfigurator_ (new HandlersConfigurator)
	{
		Handlers_ << ConcreteHandlerBase_ptr (new SystemTrayHandler);
		
		Q_FOREACH (ConcreteHandlerBase_ptr handler, Handlers_)
			handler->SetGeneralHandler (this);
			
		Cat2IconName_ ["org.LC.AdvNotifications.IM"] = "message";
	}
	
	void GeneralHandler::Handle (const Entity& e)
	{
		const QSet<ConcreteHandlerBase::HandlerType>& types = HandlersConfigurator_->GetEnabledHandlers (e);
		Q_FOREACH (ConcreteHandlerBase_ptr handler, Handlers_)
			if (types.contains (handler->GetHandlerType ()))
				handler->Handle (e);
	}
	
	HandlersConfigurator* GeneralHandler::GetHandlersConfigurator () const
	{
		return HandlersConfigurator_;
	}
	
	QIcon GeneralHandler::GetIconForCategory (const QString& cat) const
	{
		const QString& name = "notificationcategory_" + Cat2IconName_.value (cat, "general");
		return Proxy_->GetIcon (name);
	}
}
}
