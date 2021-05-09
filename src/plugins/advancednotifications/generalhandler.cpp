/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "generalhandler.h"
#include <QAction>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "systemtrayhandler.h"
#include "visualhandler.h"
#include "audiohandler.h"
#include "cmdrunhandler.h"
#include "wmurgenthandler.h"
#include "rulesmanager.h"
#include "unhandlednotificationskeeper.h"
#include "fields.h"

namespace LC
{
namespace AdvancedNotifications
{
	GeneralHandler::GeneralHandler (RulesManager *rm, const AudioThemeManager *mgr, UnhandledNotificationsKeeper *keeper)
	: RulesManager_ { rm }
	, UnhandledKeeper_ { keeper }
	{
		const auto sysTrayHandler = std::make_shared<SystemTrayHandler> ();
		Handlers_ =
		{
			sysTrayHandler,
			std::make_shared<VisualHandler> (),
			std::make_shared<AudioHandler> (mgr),
			std::make_shared<CmdRunHandler> (),
			std::make_shared<WMUrgentHandler> ()
		};

		connect (sysTrayHandler.get (),
				&SystemTrayHandler::gotActions,
				this,
				&GeneralHandler::gotActions);
	}

	void GeneralHandler::RegisterHandler (const INotificationHandler_ptr& handler)
	{
		Handlers_ << handler;
	}

	void GeneralHandler::Handle (const Entity& e)
	{
		if (e.Mime_ == "x-leechcraft/notification-rule-create")
		{
			RulesManager_->HandleEntity (e);
			return;
		}

		if (e.Additional_ [Fields::EventCategory] == Fields::Values::CancelEvent)
		{
			for (const auto& handler : Handlers_)
				handler->Handle (e, NotificationRule {});
			return;
		}

		bool wasHandled = false;
		for (const auto& rule : RulesManager_->GetRules (e))
		{
			const auto& methods = rule.GetMethods ();
			for (const auto& handler : Handlers_)
			{
				if (!(methods & handler->GetHandlerMethod ()))
					continue;

				handler->Handle (e, rule);
				wasHandled = true;
			}
		}

		if (!wasHandled)
			UnhandledKeeper_->AddUnhandled (e);
	}
}
}
