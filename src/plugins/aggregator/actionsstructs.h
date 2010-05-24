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

#ifndef PLUGINS_AGGREGATOR_ACTIONSSTRUCTS_H
#define PLUGINS_AGGREGATOR_ACTIONSSTRUCTS_H
#include <QCoreApplication>

class QAction;
class QWidget;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct AppWideActions
			{
				Q_DECLARE_TR_FUNCTIONS (AppWideActions);
			public:
				QAction *ActionAddFeed_;
				QAction *ActionUpdateFeeds_;
				QAction *ActionRegexpMatcher_;
				QAction *ActionImportOPML_;
				QAction *ActionExportOPML_;
				QAction *ActionImportBinary_;
				QAction *ActionExportBinary_;
				QAction *ActionExportFB2_;

				void SetupActionsStruct (QWidget*);
			};

			struct ChannelActions
			{
				Q_DECLARE_TR_FUNCTIONS (ChannelActions);
			public:
				QAction *ActionRemoveFeed_;
				QAction *ActionUpdateSelectedFeed_;
				QAction *ActionMarkChannelAsRead_;
				QAction *ActionMarkChannelAsUnread_;
				QAction *ActionChannelSettings_;

				void SetupActionsStruct (QWidget*);
			};
		};
	};
};

#endif

