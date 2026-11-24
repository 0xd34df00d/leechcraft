/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include "interfaces/azoth/azothcommon.h"

namespace LC::Azoth
{
	class ChatTabsManager;
	class EntriesManager;
	class NotificationsManager;
}

namespace LC::Azoth::Notifications
{
	class TypingNotifier : public QObject
	{
		const ChatTabsManager& Tabs_;
		NotificationsManager& Notifications_;

		QHash<GlobalStrongestId, bool> ShouldNotifyNext_;
	public:
		explicit TypingNotifier (const EntriesManager&, const ChatTabsManager&, NotificationsManager&, QObject* = nullptr);
	private:
		void HandleChatPartStateChanged (ICLEntry&, ChatPartState, const QString&);
	};
}
