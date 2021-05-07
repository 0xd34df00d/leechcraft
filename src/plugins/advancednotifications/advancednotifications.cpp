/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "advancednotifications.h"
#include <QIcon>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/sys/resourceloader.h>
#include <util/util.h>
#include "generalhandler.h"
#include "xmlsettingsmanager.h"
#include "notificationruleswidget.h"
#include "rulesmanager.h"
#include "quarkproxy.h"
#include "audiothememanager.h"
#include "unhandlednotificationskeeper.h"
#include "fields.h"
#include "interfaces/advancednotifications/inotificationbackendplugin.h"

namespace LC
{
namespace AdvancedNotifications
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("advancednotifications");

		Proxy_ = proxy;

		RulesManager_ = new RulesManager { proxy, this };

		auto audioThemeMgr = new AudioThemeManager { this };

		auto unhandledKeeper = new UnhandledNotificationsKeeper { proxy, this };

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"advancednotificationssettings.xml");
		SettingsDialog_->SetCustomWidget ("RulesWidget",
				new NotificationRulesWidget { RulesManager_, audioThemeMgr, unhandledKeeper, proxy });
		SettingsDialog_->SetDataSource ("AudioTheme", audioThemeMgr->GetSettingsModel ());

		GeneralHandler_ = std::make_shared<GeneralHandler> (RulesManager_, audioThemeMgr, unhandledKeeper, proxy);
		connect (GeneralHandler_.get (),
				SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)),
				this,
				SIGNAL (gotActions (QList<QAction*>, LC::ActionsEmbedPlace)));

		Component_ = std::make_shared<QuarkComponent> ("advancednotifications", "ANQuark.qml");
		Component_->StaticProps_.push_back ({ "AN_quarkTooltip", tr ("Toggle Advanced Notifications rules...") });
		Component_->DynamicProps_.push_back ({ "AN_rulesManager", RulesManager_ });
		Component_->DynamicProps_.push_back ({ "AN_proxy", new QuarkProxy });

		connect (RulesManager_,
				SIGNAL (rulesChanged ()),
				this,
				SIGNAL (rulesChanged ()));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.AdvancedNotifications";
	}

	void Plugin::Release ()
	{
		GeneralHandler_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Advanced Notifications";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Module for the advanced notifications framework.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool can = e.Mime_.startsWith ("x-leechcraft/notification") &&
				e.Additional_.contains (Fields::SenderID) &&
				e.Additional_.contains (Fields::EventID) &&
				e.Additional_.contains (Fields::EventCategory);

		if (!can)
			return EntityTestHandleResult ();

		EntityTestHandleResult result (EntityTestHandleResult::PIdeal);
		result.CancelOthers_ = true;
		return result;
	}

	void Plugin::Handle (Entity e)
	{
		GeneralHandler_->Handle (e);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { Component_ };
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << GetUniqueID () + ".NotificationsBackend";
		return result;
	}

	void Plugin::AddPlugin (QObject *obj)
	{
		const auto ip2 = qobject_cast<IPlugin2*> (obj);
		const auto& classes = ip2->GetPluginClasses ();

		if (classes.contains (GetUniqueID () + ".NotificationsBackend"))
		{
			const auto inbp = qobject_cast<INotificationBackendPlugin*> (obj);
			for (const auto& handler : inbp->GetNotificationHandlers ())
				GeneralHandler_->RegisterHandler (handler);
		}
	}

	QList<Entity> Plugin::GetAllRules (const QString& category) const
	{
		return RulesManager_->GetAllRules (category);
	}

	void Plugin::RequestRuleConfiguration (const Entity& rule)
	{
		RulesManager_->SuggestRuleConfiguration (rule);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_advancednotifications, LC::AdvancedNotifications::Plugin);
