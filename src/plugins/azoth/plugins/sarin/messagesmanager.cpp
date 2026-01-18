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
#include "util.h"

namespace LC::Azoth::Sarin
{
	MessagesManager::MessagesManager (ToxAccount *acc)
	: QObject { acc }
	{
		connect (acc,
				&ToxAccount::threadChanged,
				this,
				&MessagesManager::SetThread);
	}

	Util::ContextTask<void> MessagesManager::SendMessage (QByteArray pkey, QPointer<ChatMessage> msg)
	{
		co_await Util::AddContextObject { *this };
		const auto& body = msg->GetBody ();

		const auto runner = Runner_.lock ();
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
		const auto& val = MsgId2Msg_.take (msgId);
		if (!val)
		{
			qWarning () << "the message for ID" << msgId << "is dead";
			return;
		}
		val->SetDelivered ();
	}

	Util::ContextTask<void> MessagesManager::HandleInMessage (qint32 friendId, const QString& msg)
	{
		co_await Util::AddContextObject { *this };

		const auto runner = Runner_.lock ();
		if (!runner)
		{
			qWarning () << "got message in offline, that's kinda strange";
			co_return;
		}

		const auto& pubkey = co_await runner->Run (&ToxW::GetFriendPubkey, friendId);
		if (!pubkey)
		{
			qWarning () << "cannot get pubkey for message" << msg;
			co_return;
		}

		emit gotMessage (*pubkey, msg);
	}

	void MessagesManager::SetThread (const std::shared_ptr<ToxRunner>& runner)
	{
		Runner_ = runner;
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
