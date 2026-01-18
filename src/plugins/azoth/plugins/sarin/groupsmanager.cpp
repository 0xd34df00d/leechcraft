/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupsmanager.h"
#include <tox/tox.h>
#include <util/threads/coro.h>
#include "groupchatentry.h"
#include "toxaccount.h"
#include "toxthread.h"

namespace LC::Azoth::Sarin
{
	GroupsManager::GroupsManager (ToxAccount& acc)
	: Acc_ { acc }
	{
	}

	ToxAccount& GroupsManager::GetAccount ()
	{
		return Acc_;
	}

	void GroupsManager::HandleToxThreadChanged (const std::shared_ptr<ToxRunner>& runner)
	{
		if (!runner)
			return;

		auto route = [this] (auto fun)
		{
			return [this, fun]<typename... Args> (uint32_t groupNum, const Args&... args)
			{
				const auto groupEntry = Groups_.value (groupNum);
				if (!groupEntry)
				{
					qWarning () << "no entry for group" << groupNum;
					return;
				}

				std::invoke (fun, groupEntry, args...);
			};
		};

		connect (&*runner,
				&ToxRunner::groupPeerJoined,
				this,
				route (&GroupChatEntry::HandlePeerJoined));
	}

	Util::ContextTask<GroupsManager::JoinResult> GroupsManager::Join (QString groupId, QString nick, QString password)
	{
		co_await Util::AddContextObject { *this };

		auto tox = Acc_.GetTox ();
		if (!tox)
			co_return Util::Left { JoinGroupError::ToxOffline };

		const auto& groupIdUtf8 = groupId.toUtf8 ();
		if (groupIdUtf8.size () != TOX_GROUP_CHAT_ID_SIZE)
		{
			qWarning () << "invalid group ID length" << groupIdUtf8.size ();
			co_return Util::Left { JoinGroupError::InvalidGroupIdLength };
		}

		std::array<uint8_t, TOX_GROUP_CHAT_ID_SIZE> groupIdArr {};
		std::copy_n (groupIdUtf8.begin (), TOX_GROUP_CHAT_ID_SIZE, groupIdArr.begin ());

		const auto nickUtf8 = nick.toUtf8 ();
		const auto pwUtf8 = password.toUtf8 ();
		const auto joinResult = co_await tox->RunWithError (&tox_group_join,
				&groupIdArr [0],
				std::bit_cast<const uint8_t*> (nickUtf8.constData ()), nickUtf8.size (),
				std::bit_cast<const uint8_t*> (pwUtf8.constData ()), pwUtf8.size ());
		const auto groupNum = co_await joinResult;

		const auto entry = new GroupChatEntry { nick, groupNum, groupId, *this };
		Groups_ [groupNum] = entry;
		emit Acc_.gotCLItems ({ entry });

		co_return Util::Void {};
	}

	void GroupsManager::HandleLeft (uint32_t groupNum)
	{
		const auto entry = Groups_.take (groupNum);
		if (!entry)
		{
			qWarning () << "no entry for group" << groupNum;
			return;
		}

		auto removed = entry->GetParticipants ();
		removed << entry;
		emit Acc_.removedCLItems (removed);

		entry->deleteLater ();
	}
}
