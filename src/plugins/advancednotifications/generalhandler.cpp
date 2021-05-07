/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "generalhandler.h"
#include <interfaces/an/constants.h>
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
	GeneralHandler::GeneralHandler (RulesManager *rm, const AudioThemeManager *mgr,
			UnhandledNotificationsKeeper *keeper, const ICoreProxy_ptr& proxy)
	: RulesManager_ { rm }
	, UnhandledKeeper_ { keeper }
	, Proxy_ { proxy }
	{
		const QList<ConcreteHandlerBase_ptr> coreHandlers
		{
			std::make_shared<SystemTrayHandler> (),
			std::make_shared<VisualHandler> (),
			std::make_shared<AudioHandler> (mgr),
			std::make_shared<CmdRunHandler> (),
			std::make_shared<WMUrgentHandler> ()
		};

		for (const auto& handler : coreHandlers)
		{
			handler->SetGeneralHandler (this);
			Handlers_ << handler;
		}

		connect (coreHandlers.first ().get (),
				SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)),
				this,
				SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)));

		Cat2IconName_ =
		{
			{ AN::CatDownloads, "folder-downloads" },
			{ AN::CatIM, "mail-unread-new" },
			{ AN::CatOrganizer, "view-calendar" },
			{ AN::CatGeneric, "preferences-desktop-notification-bell" },
			{ AN::CatPackageManager, "system-software-update" },
			{ AN::CatMediaPlayer, "applications-multimedia" },
			{ AN::CatTerminal, "utilities-terminal" },
			{ AN::CatNews, "view-pim-news" }
		};
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

	ICoreProxy_ptr GeneralHandler::GetProxy () const
	{
		return Proxy_;
	}

	QIcon GeneralHandler::GetIconForCategory (const QString& cat) const
	{
		auto name = Cat2IconName_.value (cat);
		if (name.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no icon for category"
					<< cat;
			name = "dialog-information";
		}

		return Proxy_->GetIconThemeManager ()->GetIcon (name);
	}
}
}
