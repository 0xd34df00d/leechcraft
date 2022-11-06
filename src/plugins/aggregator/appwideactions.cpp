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
#include "addfeeddialog.h"
#include "dbupdatethread.h"
#include "dbutils.h"
#include "exportutils.h"
#include "updatesmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	AppWideActions::AppWideActions (const Deps& deps, QObject *parent)
	: QObject { parent }
	, ToolsMenu_ { "Aggregator" }
	{
		ToolsMenu_.setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		const auto makeAction = [&] (const QString& title, const QByteArray& iconName)
		{
			const auto action = new QAction { title, parent };
			action->setProperty ("ActionIcon", iconName);
			ToolsMenu_.addAction (action);
			AllActions_ << action;
			return action;
		};

		const auto addFeed = makeAction (tr ("Add feed..."), "list-add");
		const auto updateFeeds = makeAction (tr ("Update all feeds"), "mail-receive");
		ToolsMenu_.addSeparator ();
		const auto markAllAsRead = makeAction (tr ("Mark all channels as read"), "mail-mark-read");
		ToolsMenu_.addSection (tr ("Import/export"));
		const auto importOpml = makeAction (tr ("Import from OPML..."), "document-import");
		const auto exportOpml = makeAction (tr ("Export to OPML..."), "document-export");
		const auto exportFb2 = makeAction (tr ("Export to FB2..."), "application-xml");

		FastActions_ = { addFeed, updateFeeds };

		deps.ShortcutManager_.RegisterActions ({
					{ "ActionAddFeed", addFeed },
					{ "ActionUpdateFeeds_", updateFeeds },
					{ "ActionImportOPML_", importOpml },
					{ "ActionExportOPML_", exportOpml },
					{ "ActionExportFB2_", exportFb2 },
				});

		connect (addFeed,
				&QAction::triggered,
				this,
				[deps]
				{
					AddFeedDialog af;
					if (af.exec () == QDialog::Accepted)
						AddFeed ({
									.URL_ = af.GetURL (),
									.Tags_ = af.GetTags (),
									.UpdatesManager_ = deps.UpdatesManager_
								});
				});
		connect (updateFeeds,
				&QAction::triggered,
				&deps.UpdatesManager_,
				&UpdatesManager::UpdateFeeds);
		connect (markAllAsRead,
				&QAction::triggered,
				this,
				[deps]
				{
					if (ConfirmWithPersistence ("ConfirmMarkAllAsRead", tr ("Do you really want to mark all channels as read?")))
						deps.DBUpThread_.SetAllChannelsRead ();
				});
		// TODO importOpml
		connect (exportOpml,
				&QAction::triggered,
				[] { ExportUtils::RunExportOPML (); });
		connect (exportFb2,
				&QAction::triggered,
				[deps] { ExportUtils::RunExportFB2 (deps.ChannelsModel_); });
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
