/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
	namespace Plugins
	{
		namespace Aggregator
		{
			void AppWideActions::SetupActionsStruct (QWidget *parent)
			{
				ActionAddFeed_ = new QAction (tr ("Add feed..."),
						parent);
				ActionAddFeed_->setObjectName ("ActionAddFeed_");
				ActionAddFeed_->setProperty ("ActionIcon", "aggregator_add");

				ActionUpdateFeeds_ = new QAction (tr ("Update all feeds"),
						parent);
				ActionUpdateFeeds_->setProperty ("ActionIcon", "fetchall");

				ActionRegexpMatcher_ = new QAction (tr ("Regexp matcher..."),
						parent);
				ActionRegexpMatcher_->setObjectName ("ActionRegexpMatcher_");
				ActionRegexpMatcher_->setProperty ("ActionIcon", "aggregator_filter");

				ActionImportOPML_ = new QAction (tr ("Import from OPML..."),
						parent);
				ActionImportOPML_->setObjectName ("ActionImportOPML_");
				ActionImportOPML_->setProperty ("ActionIcon", "aggregator_importopml");

				ActionExportOPML_ = new QAction (tr ("Export to OPML..."),
						parent);
				ActionExportOPML_->setObjectName ("ActionExportOPML_");
				ActionExportOPML_->setProperty ("ActionIcon", "aggregator_exportopml");

				ActionImportBinary_ = new QAction (tr ("Import from binary..."),
						parent);
				ActionImportBinary_->setObjectName ("ActionImportBinary_");
				ActionImportBinary_->setProperty ("ActionIcon", "aggregator_importbinary");

				ActionExportBinary_ = new QAction (tr ("Export to binary..."),
						parent);
				ActionExportBinary_->setObjectName ("ActionExportBinary_");
				ActionExportBinary_->setProperty ("ActionIcon", "aggregator_exportbinary");

				ActionExportFB2_ = new QAction (tr ("Export to FB2..."),
						parent);
				ActionExportFB2_->setObjectName ("ActionExportFB2_");
				ActionExportFB2_->setProperty ("ActionIcon", "aggregator_fb2");
			}

			void ChannelActions::SetupActionsStruct (QWidget *parent)
			{
				ActionRemoveFeed_ = new QAction (tr ("Remove feed"),
						parent);
				ActionRemoveFeed_->setObjectName ("ActionRemoveFeed_");
				ActionRemoveFeed_->setProperty ("ActionIcon", "aggregator_remove");

				ActionUpdateSelectedFeed_ = new QAction (tr ("Update selected feed"),
						parent);
				ActionUpdateSelectedFeed_->setObjectName ("ActionUpdateSelectedFeed_");
				ActionUpdateSelectedFeed_->setProperty ("ActionIcon", "aggregator_updateselectedfeed");

				ActionMarkChannelAsRead_ = new QAction (tr ("Mark channel as read"),
						parent);
				ActionMarkChannelAsRead_->setObjectName ("ActionMarkChannelAsRead_");

				ActionMarkChannelAsUnread_ = new QAction (tr ("Mark channel as unread"),
						parent);
				ActionMarkChannelAsUnread_->setObjectName ("ActionMarkChannelAsUnread_");

				ActionChannelSettings_ = new QAction (tr ("Settings..."),
						parent);
				ActionChannelSettings_->setObjectName ("ActionChannelSettings_");
			}
		};
	};
};

