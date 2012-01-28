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

#include "actionsstructs.h"
#include <QAction>
#include "aggregator.h"

namespace LeechCraft
{
namespace Aggregator
{
	void AppWideActions::SetupActionsStruct (QWidget *parent)
	{
		ActionAddFeed_ = new QAction (tr ("Add feed..."),
				parent);
		ActionAddFeed_->setObjectName ("ActionAddFeed_");
		ActionAddFeed_->setProperty ("ActionIcon", "list-add");

		ActionUpdateFeeds_ = new QAction (tr ("Update all feeds"),
				parent);
		ActionUpdateFeeds_->setProperty ("ActionIcon", "mail-receive");

		ActionRegexpMatcher_ = new QAction (tr ("Regexp matcher..."),
				parent);
		ActionRegexpMatcher_->setObjectName ("ActionRegexpMatcher_");
		ActionRegexpMatcher_->setProperty ("ActionIcon", "view-filter");

		ActionImportOPML_ = new QAction (tr ("Import from OPML..."),
				parent);
		ActionImportOPML_->setObjectName ("ActionImportOPML_");
		ActionImportOPML_->setProperty ("ActionIcon", "document-import");

		ActionExportOPML_ = new QAction (tr ("Export to OPML..."),
				parent);
		ActionExportOPML_->setObjectName ("ActionExportOPML_");
		ActionExportOPML_->setProperty ("ActionIcon", "document-export");

		ActionImportBinary_ = new QAction (tr ("Import from binary..."),
				parent);
		ActionImportBinary_->setObjectName ("ActionImportBinary_");
		ActionImportBinary_->setProperty ("ActionIcon", "svn-update");

		ActionExportBinary_ = new QAction (tr ("Export to binary..."),
				parent);
		ActionExportBinary_->setObjectName ("ActionExportBinary_");
		ActionExportBinary_->setProperty ("ActionIcon", "svn-commit");

		ActionExportFB2_ = new QAction (tr ("Export to FB2..."),
				parent);
		ActionExportFB2_->setObjectName ("ActionExportFB2_");
		ActionExportFB2_->setProperty ("ActionIcon", "application-xml");
	}

	void ChannelActions::SetupActionsStruct (QWidget *parent)
	{
		ActionRemoveFeed_ = new QAction (tr ("Remove feed"),
				parent);
		ActionRemoveFeed_->setObjectName ("ActionRemoveFeed_");
		ActionRemoveFeed_->setProperty ("ActionIcon", "list-remove");

		ActionUpdateSelectedFeed_ = new QAction (tr ("Update selected feed"),
				parent);
		ActionUpdateSelectedFeed_->setObjectName ("ActionUpdateSelectedFeed_");
		ActionUpdateSelectedFeed_->setProperty ("ActionIcon", "view-refresh");

		ActionMarkChannelAsRead_ = new QAction (tr ("Mark channel as read"),
				parent);
		ActionMarkChannelAsRead_->setObjectName ("ActionMarkChannelAsRead_");
		ActionMarkChannelAsRead_->setProperty ("ActionIcon", "mail-mark-read");

		ActionMarkChannelAsUnread_ = new QAction (tr ("Mark channel as unread"),
				parent);
		ActionMarkChannelAsUnread_->setObjectName ("ActionMarkChannelAsUnread_");
		ActionMarkChannelAsUnread_->setProperty ("ActionIcon", "mail-mark-unread");

		ActionRemoveChannel_ = new QAction (tr ("Remove channel"),
				parent);
		ActionRemoveChannel_->setObjectName ("ActionRemoveChannel_");

		ActionChannelSettings_ = new QAction (tr ("Settings..."),
				parent);
		ActionChannelSettings_->setObjectName ("ActionChannelSettings_");
		ActionChannelSettings_->setProperty ("ActionIcon", "configure");
	}
}
}
