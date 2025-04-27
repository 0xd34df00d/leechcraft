/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "appwideactions.h"
#include <QAction>
#include <QMessageBox>
#include <QPushButton>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/iactionsexporter.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/qtutil.h>
#include "components/importexport/exportutils.h"
#include "components/importexport/opmladder.h"
#include "addfeeddialog.h"
#include "dbupdatethread.h"
#include "dbutils.h"
#include "updatesmanager.h"
#include "xmlsettingsmanager.h"
#include "actiondefshelpers.h"

namespace LC::Aggregator
{
	MAKE_ACTIONS (AppWideActions,
		AddFeed,
		UpdateFeeds,
		MarkAllChannelsRead,
		ImportOPML,
		ExportOPML,
		ExportFB2
	);

	namespace
	{
		ActionInfo MakeInfo (const QString& text, const QByteArray& icon)
		{
			return { .Text_ = text, .Icon_ = icon };
		}

		ActionInfo GetActionInfo (AppWideActions::ActionId action)
		{
			using enum AppWideActions::ActionId;
			switch (action)
			{
			case AddFeed:
				return MakeInfo (AppWideActions::tr ("Add feed..."), "list-add");
			case UpdateFeeds:
				return MakeInfo (AppWideActions::tr ("Update all feeds"), "mail-receive");
			case MarkAllChannelsRead:
				return MakeInfo (AppWideActions::tr ("Mark all channels as read"), "mail-mark-read");
			case ImportOPML:
				return MakeInfo (AppWideActions::tr ("Import from OPML..."), "document-import");
			case ExportOPML:
				return MakeInfo (AppWideActions::tr ("Export to OPML..."), "document-export");
			case ExportFB2:
				return MakeInfo (AppWideActions::tr ("Export to FB2..."), "application-xml");
			}

			qWarning () << "unknown action" << static_cast<int> (action);
			return {};
		}

		void RunAddFeed (UpdatesManager& um)
		{
			AddFeedDialog af;
			if (af.exec () == QDialog::Accepted)
				Aggregator::AddFeed ({
						.URL_ = af.GetURL (),
						.Tags_ = af.GetTags (),
						.UpdatesManager_ = um,
					});
		}
	}

	void AppWideActions::RegisterActions (Util::ShortcutManager& sm)
	{
		for (const auto actionId : AllActionIds ())
			sm.RegisterActionInfo (ToString (actionId), GetActionInfo (actionId));
	}

	AppWideActions::AppWideActions (const Deps& deps, QObject *parent)
	: QObject { parent }
	, ToolsMenu_ { "Aggregator"_qs }
	{
		ToolsMenu_.setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		const auto makeAction = [&] (ActionId actionId, auto handler)
		{
			const auto action = new QAction { parent };
			deps.ShortcutManager_.RegisterAction (ToString (actionId), action);
			ToolsMenu_.addAction (action);
			AllActions_ << action;

			connect (action,
					&QAction::triggered,
					this,
					handler);

			return action;
		};

		using enum ActionId;
		auto& um = deps.UpdatesManager_;
		FastActions_ << makeAction (AddFeed, [&um] { RunAddFeed (um); });
		FastActions_ << makeAction (UpdateFeeds, [&um] { um.UpdateFeeds (); });
		ToolsMenu_.addSeparator ();
		makeAction (MarkAllChannelsRead,
				[&dbup = deps.DBUpThread_]
				{
					if (ConfirmWithPersistence ("ConfirmMarkAllAsRead", tr ("Do you really want to mark all channels as read?")))
						dbup.SetAllChannelsRead ();
				});
		ToolsMenu_.addSection (tr ("Import/export"));
		makeAction (ImportOPML, [&um] { Opml::HandleOpmlFile ({}, um); });
		makeAction (ExportOPML, [] { ExportUtils::RunExportOPML (); });
		makeAction (ExportFB2, [&cm = deps.ChannelsModel_] { ExportUtils::RunExportFB2 (cm); });

		GetProxyHolder ()->GetIconThemeManager ()->UpdateIconset (AllActions_);
	}

	QList<QAction*> AppWideActions::GetActions (ActionsEmbedPlace place) const
	{
		switch (place)
		{
		case ActionsEmbedPlace::ToolsMenu:
			return { ToolsMenu_.menuAction () };
		case ActionsEmbedPlace::CommonContextMenu:
			return FastActions_;
		default:
			return {};
		}
	}

	QList<QAction*> AppWideActions::GetFastActions () const
	{
		return FastActions_;
	}

	void AppWideActions::SetEnabled (bool enabled)
	{
		for (const auto action : AllActions_)
			action->setEnabled (enabled);
	}
}
