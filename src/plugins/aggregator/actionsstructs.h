/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>

class QAction;
class QWidget;
class QMenu;

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace Aggregator
{
	struct AppWideActions
	{
		QAction *ActionAddFeed_ = nullptr;
		QAction *ActionUpdateFeeds_ = nullptr;
		QAction *ActionImportOPML_ = nullptr;
		QAction *ActionExportOPML_ = nullptr;
		QAction *ActionExportFB2_ = nullptr;
		QAction *ActionMarkAllAsRead_ = nullptr;

		explicit AppWideActions (Util::ShortcutManager*, QObject*);

		AppWideActions () = delete;
		AppWideActions (const AppWideActions&) = delete;
		AppWideActions (AppWideActions&&) = delete;
		AppWideActions& operator= (const AppWideActions&) = delete;
		AppWideActions& operator= (AppWideActions&&) = delete;

		QMenu* CreateToolMenu () const;
		void SetEnabled (bool);

		Q_DECLARE_TR_FUNCTIONS (AppWideActions)
	};

	struct ChannelActions
	{
		QAction *ActionRemoveFeed_ = nullptr;
		QAction *ActionUpdateSelectedFeed_ = nullptr;
		QAction *ActionRenameFeed_ = nullptr;
		QAction *ActionMarkChannelAsRead_ = nullptr;
		QAction *ActionMarkChannelAsUnread_ = nullptr;
		QAction *ActionRemoveChannel_ = nullptr;
		QAction *ActionChannelSettings_ = nullptr;

		explicit ChannelActions (Util::ShortcutManager*, QObject*);

		ChannelActions () = delete;
		ChannelActions (const ChannelActions&) = delete;
		ChannelActions (ChannelActions&&) = delete;
		ChannelActions& operator= (const ChannelActions&) = delete;
		ChannelActions& operator= (ChannelActions&&) = delete;

		Q_DECLARE_TR_FUNCTIONS (ChannelActions)
	};

	QMenu* CreateFeedsContextMenu (const ChannelActions&, const AppWideActions&);
}
}
