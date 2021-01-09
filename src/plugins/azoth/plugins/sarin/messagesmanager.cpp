/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "messagesmanager.h"
#include <QTimer>
#include <tox/tox.h>
#include "toxaccount.h"
#include "toxthread.h"
#include "chatmessage.h"
#include "util.h"
#include "callbackmanager.h"

namespace LC::Azoth::Sarin
{
	MessagesManager::MessagesManager (ToxAccount *acc)
	: QObject { acc }
	{
		connect (acc,
				&ToxAccount::threadChanged,
				this,
				&MessagesManager::SetThread);

		connect (this,
				&MessagesManager::invokeHandleInMessage,
				this,
				&MessagesManager::HandleInMessage);
		connect (this,
				&MessagesManager::invokeHandleReadReceipt,
				this,
				&MessagesManager::HandleReadReceipt);
	}

	void MessagesManager::SendMessage (const QByteArray& privkey, ChatMessage *msg)
	{
		const auto thread = Thread_.lock ();
		if (!thread)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot send messages in offline";
			return;
		}

		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty message";
			return;
		}

		const auto& body = msg->GetBody ();
		const auto future = thread->ScheduleFunction ([=] (Tox *tox) -> MessageSendResult
				{
					const auto friendNum = GetFriendId (tox, privkey);
					if (!friendNum)
					{
						qWarning () << Q_FUNC_INFO
								<< "unknown friend"
								<< privkey;
						return { 0, privkey, msg };
					}

					const auto& msgUtf8 = body.toUtf8 ();
					TOX_ERR_FRIEND_SEND_MESSAGE error {};
					const auto id = tox_friend_send_message (tox,
							*friendNum,
							TOX_MESSAGE_TYPE_NORMAL,
							reinterpret_cast<const uint8_t*> (msgUtf8.constData ()),
							msgUtf8.size (),
							&error);

					if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to send message"
								<< error;
						throw MakeCommandCodeException ("tox_friend_send_message", error);
					}

					return { id, privkey, msg };
				});

		Util::Sequence (this, future) >>
				[this] (const MessageSendResult& result)
				{
					if (!result.Result_)
					{
						qWarning () << Q_FUNC_INFO
								<< "message was not sent, resending in 5 seconds";

						QTimer::singleShot (5000, this,
								[result, this] { SendMessage (result.Privkey_, result.Msg_); });
					}
					else
						MsgId2Msg_ [result.Result_] = result.Msg_;
				};
	}

	void MessagesManager::HandleReadReceipt (quint32 msgId)
	{
		if (!MsgId2Msg_.contains (msgId))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown message ID"
					<< msgId
					<< MsgId2Msg_.keys ();
			return;
		}

		const auto& val = MsgId2Msg_.take (msgId);
		if (!val)
		{
			qWarning () << Q_FUNC_INFO
					<< "too late for roses, the message for ID"
					<< msgId
					<< "is dead";
			return;
		}

		val->SetDelivered ();
	}

	void MessagesManager::HandleInMessage (qint32 friendId, const QString& msg)
	{
		const auto thread = Thread_.lock ();
		if (!thread)
		{
			qWarning () << Q_FUNC_INFO
					<< "got message in offline, that's kinda strange";
			return;
		}

		Util::Sequence (this, thread->GetFriendPubkey (friendId)) >>
				[msg, this] (const QByteArray& pubkey)
				{
					if (pubkey.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
								<< "cannot get pubkey for message"
								<< msg;
						return;
					}

					emit gotMessage (pubkey, msg);
				};
	}

	void MessagesManager::SetThread (const std::shared_ptr<ToxThread>& thread)
	{
		Thread_ = thread;
		if (!thread)
			return;

		const auto cbMgr = thread->GetCallbackManager ();
		cbMgr->Register<tox_callback_friend_message> (this,
				[] (MessagesManager *pThis, uint32_t friendId, TOX_MESSAGE_TYPE, const uint8_t *msgData, size_t)
				{
					pThis->invokeHandleInMessage (friendId,
							QString::fromUtf8 (reinterpret_cast<const char*> (msgData)));
				});
		cbMgr->Register<tox_callback_friend_read_receipt> (this,
				[] (MessagesManager *pThis, uint32_t, uint32_t msgId) { pThis->invokeHandleReadReceipt (msgId); });
	}
}
