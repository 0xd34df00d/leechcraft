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
			void SetupActionsStruct (AppWideActions& aw, QWidget *parent)
			{
				aw.ActionAddFeed_ = new QAction (Aggregator::tr ("Add feed..."),
						parent);
				aw.ActionAddFeed_->setObjectName ("ActionAddFeed_");
				aw.ActionAddFeed_->setProperty ("ActionIcon", "aggregator_add");
			
				aw.ActionUpdateFeeds_ = new QAction (Aggregator::tr ("Update all feeds"),
						parent);
				aw.ActionUpdateFeeds_->setProperty ("ActionIcon", "aggregator_updateallfeeds");
			
				aw.ActionItemBucket_ = new QAction (Aggregator::tr ("Item bucket..."),
						parent);
				aw.ActionItemBucket_->setObjectName ("ActionItemBucket_");
				aw.ActionItemBucket_->setProperty ("ActionIcon", "aggregator_favorites");
			
				aw.ActionRegexpMatcher_ = new QAction (Aggregator::tr ("Regexp matcher..."),
						parent);
				aw.ActionRegexpMatcher_->setObjectName ("ActionRegexpMatcher_");
				aw.ActionRegexpMatcher_->setProperty ("ActionIcon", "aggregator_filter");
			
				aw.ActionImportOPML_ = new QAction (Aggregator::tr ("Import from OPML..."),
						parent);
				aw.ActionImportOPML_->setObjectName ("ActionImportOPML_");
				aw.ActionImportOPML_->setProperty ("ActionIcon", "aggregator_importopml");
			
				aw.ActionExportOPML_ = new QAction (Aggregator::tr ("Export to OPML..."),
						parent);
				aw.ActionExportOPML_->setObjectName ("ActionExportOPML_");
				aw.ActionExportOPML_->setProperty ("ActionIcon", "aggregator_exportopml");
			
				aw.ActionImportBinary_ = new QAction (Aggregator::tr ("Import from binary..."),
						parent);
				aw.ActionImportBinary_->setObjectName ("ActionImportBinary_");
				aw.ActionImportBinary_->setProperty ("ActionIcon", "aggregator_importbinary");
			
				aw.ActionExportBinary_ = new QAction (Aggregator::tr ("Export to binary..."),
						parent);
				aw.ActionExportBinary_->setObjectName ("ActionExportBinary_");
				aw.ActionExportBinary_->setProperty ("ActionIcon", "aggregator_exportbinary");

				aw.ActionExportFB2_ = new QAction (Aggregator::tr ("Export to FB2..."),
						parent);
				aw.ActionExportFB2_->setObjectName ("ActionExportFB2_");
				aw.ActionExportFB2_->setProperty ("ActionIcon", "aggregator_fb2");
			}

			void SetupActionsStruct (ChannelActions& ca, QWidget *parent)
			{
				ca.ActionRemoveFeed_ = new QAction (QObject::tr ("Remove feed"),
						parent);
				ca.ActionRemoveFeed_->setObjectName ("ActionRemoveFeed_");
				ca.ActionRemoveFeed_->setProperty ("ActionIcon", "aggregator_remove");

				ca.ActionUpdateSelectedFeed_ = new QAction (QObject::tr ("Update selected feed"),
						parent);
				ca.ActionUpdateSelectedFeed_->setObjectName ("ActionUpdateSelectedFeed_");
				ca.ActionUpdateSelectedFeed_->setProperty ("ActionIcon", "aggregator_updateselectedfeed");

				ca.ActionMarkChannelAsRead_ = new QAction (QObject::tr ("Mark channel as read"),
						parent);
				ca.ActionMarkChannelAsRead_->setObjectName ("ActionMarkChannelAsRead_");
			
				ca.ActionMarkChannelAsUnread_ = new QAction (QObject::tr ("Mark channel as unread"),
						parent);
				ca.ActionMarkChannelAsUnread_->setObjectName ("ActionMarkChannelAsUnread_");
			
				ca.ActionChannelSettings_ = new QAction (QObject::tr ("Settings..."),
						parent);
				ca.ActionChannelSettings_->setObjectName ("ActionChannelSettings_");
			}
		};
	};
};

