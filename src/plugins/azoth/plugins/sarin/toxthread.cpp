/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toxthread.h"
#include <cstring>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <QtEndian>
#include <QtDebug>
#include <QEventLoop>
#include <tox/tox.h>
#include "util.h"
#include "callmanager.h"
#include "callbackmanager.h"
#include "toxlogger.h"

namespace LC::Azoth::Sarin
{
	ToxThread::ToxThread (const QString& name,
			const QByteArray& state, const ToxAccountConfiguration& config)
	: Name_ { name }
	, ToxState_ { state }
	, Config_ (config)
	, CbMgr_ { std::make_shared<CallbackManager> () }
	, Logger_ { std::make_unique<ToxLogger> (name) }
	{
	}

	ToxThread::~ToxThread ()
	{
		if (!isRunning ())
			return;

		ShouldStop_ = true;

		wait (2000);
		if (isRunning ())
			terminate ();
	}

	namespace
	{
		template<typename T>
		auto DoTox (const QString& str, T f)
		{
			const auto& strUtf8 = str.toUtf8 ();
			return f (reinterpret_cast<const uint8_t*> (strUtf8.constData ()), strUtf8.size ());
		}

		State ToxStatus2State (int toxStatus)
		{
			switch (toxStatus)
			{
			case TOX_USER_STATUS_AWAY:
				return State::SAway;
			case TOX_USER_STATUS_BUSY:
				return State::SDND;
			case TOX_USER_STATUS_NONE:
				return State::SOnline;
			default:
				return State::SInvalid;
			}
		}

		bool SetToxStatus (Tox *tox, const EntryStatus& status)
		{
			const auto res = DoTox (status.StatusString_,
					[tox] (auto bytes, auto size)
					{
						TOX_ERR_SET_INFO error {};
						const bool res = tox_self_set_status_message (tox, bytes, size, &error);
						if (!res)
							qWarning () << Q_FUNC_INFO
									<< "unable to set status"
									<< error;
						return res;
					});

			if (!res)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to set status message";
				return false;
			}

			TOX_USER_STATUS toxStatus = TOX_USER_STATUS_NONE;
			switch (status.State_)
			{
			case SAway:
			case SXA:
				toxStatus = TOX_USER_STATUS_AWAY;
				break;
			case SDND:
				toxStatus = TOX_USER_STATUS_BUSY;
				break;
			default:
				break;
			}

			tox_self_set_status (tox, toxStatus);

			return true;
		}
	}

	CallManager* ToxThread::GetCallManager() const
	{
		return CallManager_.get ();
	}

	EntryStatus ToxThread::GetStatus () const
	{
		return Status_;
	}

	void ToxThread::SetStatus (const EntryStatus& status)
	{
		Status_ = status;

		if (IsStoppable ())
			ScheduleFunction ([status, this] (Tox *tox)
					{
						if (SetToxStatus (tox, status))
							emit statusChanged (status);
					});
	}

	void ToxThread::Stop ()
	{
		ShouldStop_ = true;
	}

	bool ToxThread::IsStoppable () const
	{
		return isRunning () && !ShouldStop_;
	}

	namespace
	{
		QByteArray GetToxAddress (Tox *tox)
		{
			std::array<uint8_t, TOX_ADDRESS_SIZE> address;
			tox_self_get_address (tox, address.data ());
			return ToxId2HR (address);
		}
	}

	QFuture<QByteArray> ToxThread::GetToxId ()
	{
		return ScheduleFunction (&GetToxAddress);
	}

	CallbackManager* ToxThread::GetCallbackManager () const
	{
		return CbMgr_.get ();
	}

	namespace
	{
		ToxThread::AddFriendResult ToxFAError2ThreadError (TOX_ERR_FRIEND_ADD addResult)
		{
			switch (addResult)
			{
			case TOX_ERR_FRIEND_ADD_TOO_LONG:
				return ToxThread::AddFriendResult::TooLong;
			case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
				return ToxThread::AddFriendResult::NoMessage;
			case TOX_ERR_FRIEND_ADD_OWN_KEY:
				return ToxThread::AddFriendResult::OwnKey;
			case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
				return ToxThread::AddFriendResult::AlreadySent;
			case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
				return ToxThread::AddFriendResult::BadChecksum;
			case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
				return ToxThread::AddFriendResult::NoSpam;
			case TOX_ERR_FRIEND_ADD_MALLOC:
				return ToxThread::AddFriendResult::NoMem;
			default:
				return ToxThread::AddFriendResult::Unknown;
			}
		}
	}

	namespace
	{
		QByteArray Hex2Bin (const QByteArray& key)
		{
			return QByteArray::fromHex (key.toLower ());
		}
	}

	QFuture<ToxThread::AddFriendResult> ToxThread::AddFriend (QByteArray toxId, QString msg)
	{
		toxId = Hex2Bin (toxId);
		if (msg.isEmpty ())
			msg = " ";

		return ScheduleFunction ([toxId, msg, this] (Tox *tox)
				{
					if (toxId.size () != TOX_ADDRESS_SIZE)
					{
						qWarning () << Q_FUNC_INFO
								<< "invalid Tox ID";
						return ToxThread::AddFriendResult::InvalidId;
					}

					const auto& msgUtf8 = msg.toUtf8 ();

					TOX_ERR_FRIEND_ADD error {};
					const auto addResult = tox_friend_add (tox,
							reinterpret_cast<const uint8_t*> (toxId.constData ()),
							reinterpret_cast<const uint8_t*> (msgUtf8.constData ()),
							msgUtf8.size (),
							&error);

					if (addResult == UINT32_MAX)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to add friend:"
								<< error;
						return ToxFAError2ThreadError (error);
					}

					SaveState ();

					emit gotFriend (addResult);
					return AddFriendResult::Added;
				});
	}

	void ToxThread::AddFriend (QByteArray toxId)
	{
		toxId = Hex2Bin (toxId);
		ScheduleFunction ([toxId, this] (Tox *tox)
				{
					if (toxId.size () != TOX_PUBLIC_KEY_SIZE)
						return;

					TOX_ERR_FRIEND_ADD error {};
					const auto addResult = tox_friend_add_norequest (tox,
							reinterpret_cast<const uint8_t*> (toxId.constData ()), &error);

					if (addResult == UINT32_MAX)
					{
						qDebug () << Q_FUNC_INFO
								<< "unable to add friend"
								<< error;
						return;
					}

					SaveState ();
					emit gotFriend (addResult);
				});
	}

	void ToxThread::RemoveFriend (const QByteArray& origId)
	{
		ScheduleFunction ([origId, this] (Tox *tox)
				{
					const auto friendNum = GetFriendId (tox, origId);
					if (!friendNum)
					{
						qWarning () << Q_FUNC_INFO
								<< "unknown friend"
								<< origId;
						return;
					}

					TOX_ERR_FRIEND_DELETE error {};
					if (!tox_friend_delete (tox, *friendNum, &error))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to delete friend"
								<< origId
								<< error;
						return;
					}

					emit removedFriend (origId);
					SaveState ();
				});
	}

	QFuture<std::optional<qint32>> ToxThread::ResolveFriendId (const QByteArray& pkey)
	{
		return ScheduleFunction ([pkey] (Tox *tox) -> std::optional<qint32>
				{
					const auto num = GetFriendId (tox, pkey);
					if (num == -1)
						return {};

					return num;
				});
	}

	QFuture<QByteArray> ToxThread::GetFriendPubkey (qint32 id)
	{
		return ScheduleFunction ([id] (Tox *tox)
				{
					return GetFriendId (tox, id);
				});
	}

	namespace
	{
		EntryStatus GetFriendStatus (Tox *tox, qint32 id)
		{
			TOX_ERR_FRIEND_QUERY stErr {};
			if (tox_friend_get_connection_status (tox, id, &stErr) == TOX_CONNECTION_NONE)
			{
				if (stErr != TOX_ERR_FRIEND_QUERY_OK)
					qWarning () << Q_FUNC_INFO
							<< "error querying friend connection status:"
							<< stErr
							<< ", but we'll return Offline anyway";

				return { SOffline, {} };
			}

			QString statusStr;
			const auto statusMsgSize = tox_friend_get_status_message_size (tox, id, &stErr);
			if (statusMsgSize != SIZE_MAX)
			{
				const auto statusMsg = std::make_unique<uint8_t []> (statusMsgSize);
				if (tox_friend_get_status_message (tox, id, statusMsg.get (), &stErr))
					statusStr = QString::fromUtf8 (reinterpret_cast<char*> (statusMsg.get ()), statusMsgSize);
				else
					qWarning () << Q_FUNC_INFO
							<< "unable to get status text with error"
							<< stErr;
			}

			const auto status = tox_friend_get_status (tox, id, &stErr);
			if (stErr != TOX_ERR_FRIEND_QUERY_OK)
				qWarning () << Q_FUNC_INFO
						<< "error querying friend status:"
						<< stErr;

			return
				{
					ToxStatus2State (status),
					statusStr
				};
		}
	}

	QFuture<ToxThread::FriendInfo> ToxThread::ResolveFriend (qint32 id)
	{
		return ScheduleFunction ([id] (Tox *tox)
				{
					if (!tox_friend_exists (tox, id))
						throw std::runtime_error ("Friend not found.");

					FriendInfo result;
					result.Pubkey_ = GetFriendId (tox, id);

					char name [TOX_MAX_NAME_LENGTH] = { 0 };

					TOX_ERR_FRIEND_QUERY error {};
					if (!tox_friend_get_name (tox, id, reinterpret_cast<uint8_t*> (name), &error))
						throw MakeCommandCodeException ("tox_friend_get_name", error);

					result.Name_ = QString::fromUtf8 (name);
					result.Status_ = GetFriendStatus (tox, id);

					return result;
				});
	}

	void ToxThread::ScheduleFunctionImpl (const std::function<void (Tox*)>& function)
	{
		QMutexLocker locker { &FQueueMutex_ };
		FQueue_ << function;
	}

	void ToxThread::SaveState ()
	{
		const auto size = tox_get_savedata_size (Tox_.get ());
		if (!size)
			return;

		QByteArray newState { static_cast<int> (size), 0 };
		tox_get_savedata (Tox_.get (), reinterpret_cast<uint8_t*> (newState.data ()));

		if (newState == ToxState_)
			return;

		ToxState_ = newState;
		emit toxStateChanged (ToxState_);
	}

	void ToxThread::LoadFriends ()
	{
		const auto count = tox_self_get_friend_list_size (Tox_.get ());
		qDebug () << Q_FUNC_INFO << count;
		if (count <= 0)
			return;

		const auto friendList = std::make_unique<uint32_t []> (count);
		tox_self_get_friend_list (Tox_.get (), friendList.get ());

		for (uint32_t i = 0; i < count; ++i)
			emit gotFriend (friendList [i]);
	}

	void ToxThread::HandleFriendRequest (const uint8_t *pkey, const uint8_t *data, size_t size)
	{
		const auto& pubkey = ToxId2HR<TOX_PUBLIC_KEY_SIZE> (pkey);
		const auto& msg = QString::fromUtf8 (reinterpret_cast<const char*> (data), size);
		qDebug () << Q_FUNC_INFO << pubkey << msg;

		emit gotFriendRequest (pubkey, msg);
	}

	void ToxThread::HandleNameChange (uint32_t id, const uint8_t *data, uint16_t)
	{
		const auto& toxId = GetFriendId (Tox_.get (), id);
		const auto& name = QString::fromUtf8 (reinterpret_cast<const char*> (data));
		qDebug () << Q_FUNC_INFO << toxId << name;
		emit friendNameChanged (toxId, name);

		SaveState ();
	}

	void ToxThread::UpdateFriendStatus (uint32_t friendId)
	{
		const auto& id = GetFriendId (Tox_.get (), friendId);
		const auto& status = GetFriendStatus (Tox_.get (), friendId);
		emit friendStatusChanged (id, status);
	}

	void ToxThread::HandleTypingChange (uint32_t friendId, bool isTyping)
	{
		const auto& id = GetFriendId (Tox_.get (), friendId);
		emit friendTypingChanged (id, isTyping);
	}

	void ToxThread::SetCallbacks ()
	{
		CbMgr_->Register<tox_callback_friend_request> (this, &ToxThread::HandleFriendRequest);
		CbMgr_->Register<tox_callback_friend_name> (this,
				[] (ToxThread *thread, uint32_t id, const uint8_t *name, size_t len)
				{
					thread->HandleNameChange (id, name, len + 1);
				});
		CbMgr_->Register<tox_callback_friend_status> (this,
				[] (ToxThread *thread, uint32_t friendId, TOX_USER_STATUS)
				{
					thread->UpdateFriendStatus (friendId);
				});
		CbMgr_->Register<tox_callback_friend_status_message> (this,
				[] (ToxThread *thread, uint32_t friendId, const uint8_t*, size_t)
				{
					thread->UpdateFriendStatus (friendId);
				});
		CbMgr_->Register<tox_callback_friend_connection_status> (this,
				[] (ToxThread *thread, uint32_t friendId, TOX_CONNECTION)
				{
					thread->UpdateFriendStatus (friendId);
				});
		CbMgr_->Register<tox_callback_friend_typing> (this, &ToxThread::HandleTypingChange);
	}

	void ToxThread::RunTox ()
	{
		Tox_Options opts
		{
			.ipv6_enabled = Config_.AllowIPv6_,
			.udp_enabled = Config_.AllowUDP_,
			.local_discovery_enabled = Config_.AllowLocalDiscovery_,
			.dht_announcements_enabled = true,		// TODO
			.proxy_type = Config_.ProxyHost_.isEmpty () ? TOX_PROXY_TYPE_NONE : TOX_PROXY_TYPE_SOCKS5,			// TODO support HTTP proxies
			.proxy_host = Config_.ProxyHost_.isEmpty () ? nullptr : strdup (Config_.ProxyHost_.toLatin1 ()),	// this leaks the string
			.proxy_port = static_cast<uint16_t> (Config_.ProxyPort_),
			.start_port = 0,			// TODO
			.end_port = 0,			// TODO
			.tcp_port = 0,			// TODO
			.hole_punching_enabled = Config_.UdpHolePunching_,
			.savedata_type = ToxState_.isEmpty () ? TOX_SAVEDATA_TYPE_NONE : TOX_SAVEDATA_TYPE_TOX_SAVE,
			.savedata_data = reinterpret_cast<const uint8_t*> (ToxState_.constData ()),
			.savedata_length = static_cast<size_t> (ToxState_.size ()),
			.log_callback = [] (Tox*,
					TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func, const char *message,
					void *udata)
			{
				static_cast<ToxLogger*> (udata)->Log (level, file, line, func, message);
			},
			.log_user_data = Logger_.get (),
			.experimental_thread_safety = false,
			.experimental_groups_persistence = false,
		};

		TOX_ERR_NEW creationError {};
		Tox_ = std::shared_ptr<Tox> { tox_new (&opts, &creationError), &tox_kill };
		if (!Tox_ || creationError != TOX_ERR_NEW_OK)
			throw MakeCommandCodeException ("tox_new", creationError);

		CbMgr_->SetTox (Tox_);

		CallManager_ = std::make_shared<CallManager> (this, Tox_.get ());

		DoTox (Name_,
				[this] (const uint8_t *bytes, uint16_t size)
				{
					TOX_ERR_SET_INFO error {};
					if (!tox_self_set_name (Tox_.get (), bytes, size, &error))
						throw MakeCommandCodeException ("tox_self_set_name", error);
				});

		SetCallbacks ();

		LoadFriends ();

		SetToxStatus (Tox_.get (), Status_);

		emit toxCreated ();

		qDebug () << "gonna bootstrap..." << Tox_.get ();
		const auto pubkey = Hex2Bin ("F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67");
		TOX_ERR_BOOTSTRAP bsError {};
		if (!tox_bootstrap (Tox_.get (),
					"192.210.149.121",
					static_cast<uint16_t> (33445),
					reinterpret_cast<const uint8_t*> (pubkey.constData ()),
					&bsError))
			throw MakeCommandCodeException ("tox_bootstrap", bsError);

		bool wasConnected = false;

		// TODO handle tox_callback_self_connection_status()
		QEventLoop evLoop;
		while (!ShouldStop_)
		{
			tox_iterate (Tox_.get (), CbMgr_.get ());
			auto next = tox_iteration_interval (Tox_.get ());

			if (!wasConnected && tox_self_get_connection_status (Tox_.get ()) != TOX_CONNECTION_NONE)
			{
				wasConnected = true;
				qDebug () << "connected! tox id is" << GetToxAddress (Tox_.get ());

				emit statusChanged (Status_);

				SaveState ();
			}

			QElapsedTimer timer;
			timer.start ();

			evLoop.processEvents ();

			decltype (FQueue_) queue;
			{
				QMutexLocker locker { &FQueueMutex_ };
				std::swap (queue, FQueue_);
			}
			for (const auto& item : queue)
				item (Tox_.get ());

			next -= timer.elapsed ();

			if (next > 0)
			{
				if (next > 100)
					next = 50;

				msleep (next);
			}
		}

		SaveState ();
	}

	void ToxThread::run ()
	{
		qDebug () << Q_FUNC_INFO;

		emit statusChanged ({ SConnecting, {} });

		try
		{
			RunTox ();
		}
		catch (const Util::QtException_t& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "got concurrent exception:"
					<< e.what ();
			emit fatalException (Util::QtException_ptr { e.clone () });
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "got standard exception:"
					<< e.what ();
			emit fatalException (std::make_shared<Util::ConcurrentStdException> (e));
		}
	}
}
