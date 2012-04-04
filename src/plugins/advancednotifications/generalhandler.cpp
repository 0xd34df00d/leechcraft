/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "generalhandler.h"
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include "systemtrayhandler.h"
#include "visualhandler.h"
#include "audiohandler.h"
#include "cmdrunhandler.h"
#include "core.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	GeneralHandler::GeneralHandler (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
		Handlers_ << ConcreteHandlerBase_ptr (new SystemTrayHandler);
		Handlers_ << ConcreteHandlerBase_ptr (new VisualHandler);
		Handlers_ << ConcreteHandlerBase_ptr (new AudioHandler);
		Handlers_ << ConcreteHandlerBase_ptr (new CmdRunHandler);

		Q_FOREACH (ConcreteHandlerBase_ptr handler, Handlers_)
			handler->SetGeneralHandler (this);

		connect (Handlers_.first ().get (),
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
				this,
				SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));

		Cat2IconName_ ["org.LC.AdvNotifications.IM"] = "mail-unread-new";
		Cat2IconName_ ["org.LC.AdvNotifications.Organizer"] = "view-calendar";
	}

	void GeneralHandler::Handle (const Entity& e)
	{
		if (e.Additional_ ["org.LC.AdvNotifications.EventCategory"] == "org.LC.AdvNotifications.Cancel")
		{
			Q_FOREACH (ConcreteHandlerBase_ptr handler, Handlers_)
				handler->Handle (e, NotificationRule ());
			return;
		}

		const QList<NotificationRule>& rules = Core::Instance ().GetRules (e);
		Q_FOREACH (const NotificationRule& rule, rules)
		{
			NotificationMethods methods = rule.GetMethods ();

			Q_FOREACH (ConcreteHandlerBase_ptr handler, Handlers_)
			{
				if (!(methods & handler->GetHandlerMethod ()))
					continue;

				handler->Handle (e, rule);
			}
		}
	}

	ICoreProxy_ptr GeneralHandler::GetProxy () const
	{
		return Proxy_;
	}

	QIcon GeneralHandler::GetIconForCategory (const QString& cat) const
	{
		const QString& name = Cat2IconName_.value (cat, "general");
		return Proxy_->GetIcon (name);
	}
}
}
