/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "messagesmanager.h"
#include <tox/tox.h>
#include <util/threads/coro.h>
#include "toxaccount.h"
#include "toxthread.h"
#include "chatmessage.h"
#include "toxcontact.h"
#include "util.h"

namespace LC::Azoth::Sarin
{
	MessagesManager::MessagesManager (ToxAccount& acc)
	: QObject { &acc }
	, Acc_ { acc }
	{
		connect (&acc,
				&ToxAccount::threadChanged,
				this,
				&MessagesManager::SetThread);
		SetThread (acc.GetTox ());
	}

	Util::ContextTask<void> MessagesManager::SendMessage (Pubkey pkey, QPointer<ChatMessage> msg)
	{
		co_await Util::AddContextObject { *this };
		const auto& body = msg->GetBody ();

		const auto runner = Acc_.GetTox ();
		if (!runner)
		{
			qWarning () << "cannot send messages in offline";
			co_return;
		}

		const auto friendNum = co_await runner->Run (&ToxW::ResolveFriendNum, pkey);
		if (!friendNum)
		{
			qWarning () << "unknown friend" << friendNum;
			co_return;
		}

		using namespace std::chrono_literals;
		constexpr int retries = 10;
		constexpr auto backoffBase = 1s;
		for (int i = 0; i < retries; ++i)
		{
			const auto sendResult = co_await runner->RunWithStrError (&tox_friend_send_message, body, *friendNum, TOX_MESSAGE_TYPE_NORMAL);
			// TODO more specific error handling
			Util::Visit (sendResult,
					[&, this] (uint32_t msgId) { MsgId2Msg_ [msgId] = msg; },
					[] (TOX_ERR_FRIEND_SEND_MESSAGE error) { qWarning () << "unable to send message" << error; });
			if (sendResult.IsRight ())
				break;

			co_await (backoffBase * (i + 1));
		}
	}

	void MessagesManager::HandleReadReceipt (quint32 msgId)
	{
		if (const auto& val = MsgId2Msg_.take (msgId))
			val->SetDelivered ();
		else
			qWarning () << "the message for ID" << msgId << "is dead";
	}

	Util::ContextTask<void> MessagesManager::HandleInMessage (qint32 friendId, QString body)
	{
		co_await Util::AddContextObject { *this };

		const auto runner = co_await Util::NonEmpty (Acc_.GetTox (), "got message in offline");
		const auto maybePubkey = co_await runner->Run (&ToxW::GetFriendPubkey, friendId);
		const auto pubkey = co_await Util::NonEmpty (maybePubkey, std::tie ("cannot get pubkey for message", friendId));
		auto& contact = Acc_.GetOrCreateByPubkey (*pubkey);

		const auto msg = new ChatMessage { body, IMessage::Direction::In, &contact };
		msg->Store ();
	}

	void MessagesManager::SetThread (const std::shared_ptr<ToxRunner>& runner)
	{
		if (!runner)
			return;

		connect (&*runner,
				&ToxRunner::incomingMessage,
				this,
				&MessagesManager::HandleInMessage);
		connect (&*runner,
				&ToxRunner::readReceipt,
				this,
				&MessagesManager::HandleReadReceipt);
	}
}
