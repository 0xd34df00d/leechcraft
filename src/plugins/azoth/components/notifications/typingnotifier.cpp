/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "typingnotifier.h"
#include "interfaces/azoth/iclentry.h"
#include "components/managers/entriesmanager.h"
#include "chattabsmanager.h"
#include "notificationsmanager.h"
#include "types.h"

namespace LC::Azoth::Notifications
{
	TypingNotifier::TypingNotifier (const EntriesManager& entries,
			const ChatTabsManager& tabs,
			NotificationsManager& notifications,
			QObject *parent)
	: QObject { parent }
	, Notifications_ { notifications }
	{
		connect (&entries,
				&EntriesManager::added,
				this,
				[this] (const QList<ICLEntry*>& entries)
				{
					for (const auto entry : entries)
						connect (&entry->GetCLEntryEmitter (),
								&Emitters::CLEntry::chatPartStateChanged,
								this,
								std::bind_front (&TypingNotifier::HandleChatPartStateChanged, this, std::ref (*entry)));
				});
		connect (&entries,
				&EntriesManager::strongestIdChanged,
				this,
				[this] (const GlobalConventionalId& oldId, const GlobalConventionalId& newId)
				{
					if (ShouldNotifyNext_.contains (oldId))
						ShouldNotifyNext_ [newId] = ShouldNotifyNext_.take (oldId);
				});

		connect (&tabs,
				&ChatTabsManager::entryMadeCurrent,
				this,
				[this] (const ICLEntry& entry) { ShouldNotifyNext_ [entry.GetGlobalStrongestId ()] = true; });
	}

	void TypingNotifier::HandleChatPartStateChanged (ICLEntry& entry, ChatPartState state, const QString&)
	{
		if (state != CPSComposing)
			return;

		const auto& id = entry.GetGlobalStrongestId ();
		if (!ShouldNotifyNext_.value (id, true))
			return;

		ShouldNotifyNext_ [id] = false;
		Notifications_.Notify (entry, Typing {});
	}
}
