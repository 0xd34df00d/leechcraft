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
#include <util/xpc/an/constants.h>
#include <util/xpc/an/entityfields.h>
#include <util/xpc/entityconstants.h>
#include "systemtrayhandler.h"
#include "visualhandler.h"
#include "audiohandler.h"
#include "cmdrunhandler.h"
#include "wmurgenthandler.h"
#include "rulesmanager.h"
#include "unhandlednotificationskeeper.h"

namespace LC::AdvancedNotifications
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
		if (e.Mime_ == Mimes::NotificationRuleCreate)
		{
			RulesManager_->HandleEntity (e);
			return;
		}

		if (e.Additional_ [AN::EF::EventCategory].toString () == AN::CatEventCancel)
		{
			for (const auto& handler : Handlers_)
				handler->Handle (e, {});
			return;
		}

		const auto& rules = RulesManager_->GetRules (e);

		bool wasHandled = false;
		for (const auto& handler : Handlers_)
		{
			const auto method = handler->GetHandlerMethod ();

			QList<const INotificationRule*> asking;
			for (const auto& rule : rules)
				if (rule.GetMethods () & method)
					asking << &rule;
			if (asking.isEmpty ())
				continue;

			handler->Handle (e, asking);
			wasHandled = true;
		}

		if (!wasHandled)
			UnhandledKeeper_->AddUnhandled (e);
	}
}
