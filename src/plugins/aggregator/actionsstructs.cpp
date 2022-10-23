/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "actionsstructs.h"
#include <QAction>
#include <QMenu>
#include <util/shortcuts/shortcutmanager.h>

namespace LC
{
namespace Aggregator
{
	AppWideActions::AppWideActions (Util::ShortcutManager *shortcutMgr, QObject *parent)
	{
		ActionAddFeed_ = new QAction (tr ("Add feed..."), parent);
		ActionAddFeed_->setObjectName ("ActionAddFeed_");
		ActionAddFeed_->setProperty ("ActionIcon", "list-add");

		ActionUpdateFeeds_ = new QAction (tr ("Update all feeds"), parent);
		ActionUpdateFeeds_->setProperty ("ActionIcon", "mail-receive");

		ActionImportOPML_ = new QAction (tr ("Import from OPML..."), parent);
		ActionImportOPML_->setObjectName ("ActionImportOPML_");
		ActionImportOPML_->setProperty ("ActionIcon", "document-import");

		ActionExportOPML_ = new QAction (tr ("Export to OPML..."), parent);
		ActionExportOPML_->setObjectName ("ActionExportOPML_");
		ActionExportOPML_->setProperty ("ActionIcon", "document-export");

		ActionExportFB2_ = new QAction (tr ("Export to FB2..."), parent);
		ActionExportFB2_->setObjectName ("ActionExportFB2_");
		ActionExportFB2_->setProperty ("ActionIcon", "application-xml");

		ActionMarkAllAsRead_ = new QAction (tr ("Mark all channels as read"), parent);
		ActionMarkAllAsRead_->setObjectName ("ActionMarkAllAsRead_");
		ActionMarkAllAsRead_->setProperty ("ActionIcon", "mail-mark-read");

		shortcutMgr->RegisterActions ({
					{ "ActionAddFeed", ActionAddFeed_ },
					{ "ActionUpdateFeeds_", ActionUpdateFeeds_ },
					{ "ActionImportOPML_", ActionImportOPML_ },
					{ "ActionExportOPML_", ActionExportOPML_ },
					{ "ActionExportFB2_", ActionExportFB2_ }
				});
	}

	QMenu* AppWideActions::CreateToolMenu () const
	{
		const auto menu = new QMenu (tr ("Aggregator"));
		menu->addAction (ActionMarkAllAsRead_);
		menu->addSeparator ();
		menu->addAction (ActionImportOPML_);
		menu->addAction (ActionExportOPML_);
		menu->addAction (ActionExportFB2_);
		return menu;
	}

	void AppWideActions::SetEnabled (bool enabled)
	{
		ActionAddFeed_->setEnabled (enabled);
		ActionUpdateFeeds_->setEnabled (enabled);
		ActionImportOPML_->setEnabled (enabled);
		ActionExportOPML_->setEnabled (enabled);
		ActionExportFB2_->setEnabled (enabled);
		ActionMarkAllAsRead_->setEnabled (enabled);
	}

	ChannelActions::ChannelActions (Util::ShortcutManager *shortcutMgr, QObject *parent)
	{
		ActionRemoveFeed_ = new QAction (tr ("Remove feed"), parent);
		ActionRemoveFeed_->setObjectName ("ActionRemoveFeed_");
		ActionRemoveFeed_->setProperty ("ActionIcon", "list-remove");

		ActionUpdateSelectedFeed_ = new QAction (tr ("Update selected feed"), parent);
		ActionUpdateSelectedFeed_->setObjectName ("ActionUpdateSelectedFeed_");
		ActionUpdateSelectedFeed_->setProperty ("ActionIcon", "view-refresh");

		ActionRenameFeed_ = new QAction (tr ("Rename feed"), parent);
		ActionRenameFeed_->setObjectName ("ActionRenameFeed_");
		ActionRenameFeed_->setProperty ("ActionIcon", "edit-rename");

		ActionMarkChannelAsRead_ = new QAction (tr ("Mark channel as read"), parent);
		ActionMarkChannelAsRead_->setObjectName ("ActionMarkChannelAsRead_");
		ActionMarkChannelAsRead_->setProperty ("ActionIcon", "mail-mark-read");

		ActionMarkChannelAsUnread_ = new QAction (tr ("Mark channel as unread"), parent);
		ActionMarkChannelAsUnread_->setObjectName ("ActionMarkChannelAsUnread_");
		ActionMarkChannelAsUnread_->setProperty ("ActionIcon", "mail-mark-unread");

		ActionRemoveChannel_ = new QAction (tr ("Remove channel"), parent);
		ActionRemoveChannel_->setObjectName ("ActionRemoveChannel_");

		ActionChannelSettings_ = new QAction (tr ("Settings..."), parent);
		ActionChannelSettings_->setObjectName ("ActionChannelSettings_");
		ActionChannelSettings_->setProperty ("ActionIcon", "configure");

		shortcutMgr->RegisterActions ({
					{ "ActionRemoveFeed_", ActionRemoveFeed_ },
					{ "ActionUpdateSelectedFeed_", ActionUpdateSelectedFeed_ },
					{ "ActionMarkChannelAsRead_", ActionMarkChannelAsRead_ },
					{ "ActionMarkChannelAsUnread_", ActionMarkChannelAsUnread_ },
					{ "ActionChannelSettings_", ActionChannelSettings_ }
				});
	}

	QMenu* CreateFeedsContextMenu (const ChannelActions& channelActions, const AppWideActions& appWideActions)
	{
		const auto result = new QMenu (ChannelActions::tr ("Feeds actions"));
		result->addAction (channelActions.ActionMarkChannelAsRead_);
		result->addAction (channelActions.ActionMarkChannelAsUnread_);
		result->addSeparator ();
		result->addAction (channelActions.ActionRemoveFeed_);
		result->addAction (channelActions.ActionUpdateSelectedFeed_);
		result->addAction (channelActions.ActionRenameFeed_);
		result->addSeparator ();
		result->addAction (channelActions.ActionChannelSettings_);
		result->addSeparator ();
		result->addAction (appWideActions.ActionAddFeed_);
		return result;
	}
}
}
