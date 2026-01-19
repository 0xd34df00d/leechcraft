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
#include <QtEndian>
#include <QtDebug>
#include <QEventLoop>
#include <tox/tox.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro/either.h>
#include <util/threads/coro/eithercoro.h>
#include "util.h"
#include "callmanager.h"
#include "toxlogger.h"

namespace LC::Azoth::Sarin
{
	ToxW::ToxW (const InitContext& ctx, ToxRunner& runner)
	: Name_ { ctx.Name_ }
	, Config_ { ctx.Config_ }
	, ToxState_ { ctx.State_ }
	, Logger_ { std::make_unique<ToxLogger> (ctx.Name_) }
	, Tox_ { nullptr, nullptr }
	, Runner_ { runner }
	{
	}

	ToxW::~ToxW () = default;

	Tox* ToxW::GetTox ()
	{
		return Tox_.get ();
	}

	QByteArray ToxW::GetToxId () const
	{
		std::array<uint8_t, TOX_ADDRESS_SIZE> address {};
		tox_self_get_address (Tox_.get (), address.data ());
		return ToxId2HR (address);
	}

	namespace
	{
		QByteArray Hex2Bin (const QByteArray& key)
		{
			return QByteArray::fromHex (key.toLower ());
		}
	}

	bool ToxW::SetStatus (const EntryStatus& status)
	{
		TargetStatus_ = status;
		if (!Tox_)
			return true;

		if (const auto res = WithStrError (&tox_self_set_status_message, Tox_.get (), status.StatusString_);
			res.IsLeft ())
		{
			qWarning () <<  "unable to set status message" << res.GetLeft ();
			return false;
		}

		auto toxStatus = TOX_USER_STATUS_NONE;
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
		tox_self_set_status (Tox_.get (), toxStatus);
		emit Runner_.statusChanged (status);
		return true;
	}

	namespace
	{
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
	}

	AddFriendResult ToxW::AddFriend (Pubkey pkey, QString msg)
	{
		if (msg.isEmpty ())
			msg = " ";
		const auto& msgUtf8 = msg.toUtf8 ();

		auto result = WithError (tox_friend_add,
				Tox_.get (),
				pkey.data (),
				std::bit_cast<const uint8_t*> (msgUtf8.constData ()),
				msgUtf8.size ());
		SaveState ();
		return result;
	}

	AddFriendResult ToxW::AddFriendNoRequest (Pubkey toxId)
	{
		auto result = WithError (tox_friend_add_norequest, Tox_.get (), toxId.data ());
		SaveState ();
		return result;
	}

	bool ToxW::RemoveFriend (Pubkey pkey)
	{
		const auto friendNum = GetFriendId (Tox_.get (), pkey);
		if (!friendNum)
		{
			qWarning () << "unknown friend" << pkey;
			return true;
		}

		TOX_ERR_FRIEND_DELETE error {};
		if (!tox_friend_delete (Tox_.get (), *friendNum, &error))
			return false;

		SaveState ();
		return true;
	}

	std::optional<uint32_t> ToxW::ResolveFriendNum (Pubkey pkey) const
	{
		return GetFriendId (Tox_.get (), pkey);
	}

	std::optional<Pubkey> ToxW::GetFriendPubkey (uint32_t id) const
	{
		return Sarin::GetFriendPubkey (Tox_.get (), id);
	}

	namespace
	{
		Util::Either<ToxError<FriendQueryError>, EntryStatus> GetFriendStatusEither (Tox *tox, uint32_t id)
		{
			const auto connStatus = co_await WithError (&tox_friend_get_connection_status, tox, id);
			if (connStatus == TOX_CONNECTION_NONE)
				co_return EntryStatus { SOffline, {} };

			const auto status = co_await WithError (&tox_friend_get_status, tox, id);
			const auto& statusMsg = co_await QueryToxString (&tox_friend_get_status_message_size,
					&tox_friend_get_status_message,
					tox, id);
			co_return EntryStatus { ToxStatus2State (status), statusMsg };
		}

		EntryStatus GetFriendStatus (Tox *tox, uint32_t id)
		{
			return GetFriendStatusEither (tox, id)
					.ToRight ([] (const auto& error) { return EntryStatus { SError, error.Message_ }; });
		}
	}

	ToxW::ResolveResult ToxW::ResolveFriend (uint32_t id) const
	{
		const auto pubkey = GetFriendPubkey (id);
		if (!pubkey)
			co_return Util::Left { ToxError { FriendQueryError::NotFound, "friend does not exist" } };

		FriendInfo result;
		result.Pubkey_ = *pubkey;
		result.Name_ = co_await QueryToxString (&tox_friend_get_name_size, &tox_friend_get_name, Tox_.get (), id);
		result.Status_ = GetFriendStatus (Tox_.get (), id);
		co_return result;
	}

	QList<uint32_t> ToxW::GetFriendList () const
	{
		QList<uint32_t> friendList;
		friendList.resize (tox_self_get_friend_list_size (Tox_.get ()));
		tox_self_get_friend_list (Tox_.get (), &friendList [0]);
		return friendList;
	}

	void ToxW::SaveState ()
	{
		const auto size = tox_get_savedata_size (Tox_.get ());
		if (!size)
			return;

		QByteArray newState { static_cast<int> (size), 0 };
		tox_get_savedata (Tox_.get (), reinterpret_cast<uint8_t*> (newState.data ()));

		if (newState == ToxState_)
			return;

		ToxState_ = newState;
		emit Runner_.toxStateChanged (ToxState_);
	}

	ToxRunner& ToxW::Runner (void *udata)
	{
		return static_cast<ToxW*> (udata)->Runner_;
	}

	namespace
	{
		template<typename... Args>
		using ToxCb = void (*) (Args...);

		template<auto Reg, auto Handler>
		struct RegHandler;

		template<typename... Args, void (*Reg) (Tox*, ToxCb<Args...>), auto Handler>
		struct RegHandler<Reg, Handler>
		{
			void operator() (Tox *tox) const
			{
				[&]<size_t... Ixs> (std::index_sequence<Ixs...>)
				{
					Reg (tox,
							[] (Args... [Ixs]... args, void *udata) { Handler (*static_cast<ToxW*> (udata), args...); });
				} (std::make_index_sequence<sizeof... (Args) - 1> {});
			}
		};
	}

	template<auto Reg, auto Handler>
	void ToxW::Register ()
	{
		RegHandler<Reg, Handler> {} (Tox_.get ());
	}

	void ToxW::InitializeCallbacks ()
	{
		// friend requests and statuses
		Register<tox_callback_friend_request,
				[] (ToxW& self, Tox*, const uint8_t pkeyPtr [TOX_PUBLIC_KEY_SIZE], const uint8_t *data, size_t size)
				{
					Pubkey pkey {};
					std::copy_n (pkeyPtr, pkey.size (), pkey.begin ());

					const auto& msg = FromToxStr (data, size);
					emit self.Runner_.gotFriendRequest (pkey, msg);
				}> ();
		Register<tox_callback_friend_name,
				[] (ToxW& self, Tox*, uint32_t num, const uint8_t *data, size_t len)
				{
					if (const auto& toxId = self.GetFriendPubkey (num))
					{
						const auto& name = FromToxStr (data, len);
						emit self.Runner_.friendNameChanged (*toxId, name);
						self.SaveState ();
					}
				}> ();

		constexpr auto updateFriendStatus = [] (ToxW& self, Tox *tox, uint32_t num, auto...)
		{
			if (const auto& id = self.GetFriendPubkey (num))
				emit self.Runner_.friendStatusChanged (*id, GetFriendStatus (tox, num));
		};
		Register<tox_callback_friend_status, updateFriendStatus> ();
		Register<tox_callback_friend_status_message, updateFriendStatus> ();
		Register<tox_callback_friend_connection_status, updateFriendStatus> ();

		Register<tox_callback_friend_typing,
				[] (ToxW& self, Tox*, uint32_t num, bool isTyping)
				{
					if (const auto& id = self.GetFriendPubkey (num))
						emit self.Runner_.friendTypingChanged (*id, isTyping);
				}> ();

		// self statuses
		Register<tox_callback_self_connection_status,
				[] (ToxW& self, Tox*, Tox_Connection status)
				{
					emit self.Runner_.statusChanged (status == TOX_CONNECTION_NONE ?
							EntryStatus { SConnecting, {} } :
							self.TargetStatus_);
				}> ();

		// file transfers
		Register<tox_callback_file_recv_control,
				[] (ToxW& self, Tox*, uint32_t friendNum, uint32_t fileNum, TOX_FILE_CONTROL ctrl)
				{
					emit self.Runner_.gotFileControl (friendNum, fileNum, ctrl);
				}> ();
		Register<tox_callback_file_recv,
				[] (ToxW& self, Tox*, uint32_t friendNum,
						uint32_t filenum, uint32_t kind, uint64_t filesize,
						const uint8_t *rawFilename, size_t filenameLength)
				{
					if (const auto pkey = self.GetFriendPubkey (friendNum))
						emit self.Runner_.requested (friendNum, *pkey, filenum, filesize, FromToxStr (rawFilename, filenameLength));
				}> ();
		Register<tox_callback_file_recv_chunk,
				[] (ToxW& self, Tox*, uint32_t friendNum, uint32_t fileNum, uint64_t position, const uint8_t *rawData, qsizetype rawSize)
				{
					emit self.Runner_.gotData (friendNum, fileNum, FromToxBytes (rawData, rawSize), position);
				}> ();
		Register<tox_callback_file_chunk_request,
				[] (ToxW& self, Tox*, uint32_t friendNum, uint32_t fileNum, uint64_t position, size_t length)
				{
					emit self.Runner_.gotChunkRequest (friendNum, fileNum, position, length);
				}> ();

		// messages
		Register<tox_callback_friend_message,
				[] (ToxW& self, Tox*, uint32_t friendId, TOX_MESSAGE_TYPE, const uint8_t *msg, size_t size)
				{
					emit self.Runner_.incomingMessage (friendId, FromToxStr (msg, size));
				}> ();
		Register<tox_callback_friend_read_receipt,
				[] (ToxW& self, Tox*, uint32_t, uint32_t msgId)
				{
					emit self.Runner_.readReceipt (msgId);
				}> ();

		// conferences
		Register<tox_callback_conference_invite,
				[] (ToxW& self, Tox*, uint32_t friendNum, TOX_CONFERENCE_TYPE type, const uint8_t *cookie, size_t len)
				{
					if (const auto pkey = self.GetFriendPubkey (friendNum))
						emit self.Runner_.confInvited ({
									.FriendNum_ = friendNum,
									.FriendPKey_ = *pkey,
									.Type_ = FromToxEnum (type),
									.Cookie_ = FromToxBytes (cookie, len),
								});
					else
						qWarning () << "unable to get pkey for" << friendNum;
				}> ();
		Register<tox_callback_conference_connected,
				[] (ToxW& self, Tox*, uint32_t confNum)
				{
					emit self.Runner_.confConnected (confNum);
				}> ();
		Register<tox_callback_conference_message,
				[] (ToxW& self, Tox*, uint32_t confNum, uint32_t peerNum, TOX_MESSAGE_TYPE type, const uint8_t *msg, size_t len)
				{
					emit self.Runner_.confMessage (confNum, { peerNum, FromToxEnum (type), FromToxStr (msg, len) });
				}> ();
		Register<tox_callback_conference_title,
				[] (ToxW& self, Tox*, uint32_t confNum, uint32_t peerNum, const uint8_t *title, size_t len)
				{
					emit self.Runner_.confTitleChanged (confNum, { peerNum, FromToxStr (title, len) });
				}> ();
		Register<tox_callback_conference_peer_name,
				[] (ToxW& self, Tox*, uint32_t confNum, uint32_t peerNum, const uint8_t *name, size_t len)
				{
					emit self.Runner_.confPeerNameChanged (confNum, { peerNum, FromToxStr (name, len) });
				}> ();
		Register<tox_callback_conference_peer_list_changed,
				[] (ToxW& self, Tox*, uint32_t confNum)
				{
					emit self.Runner_.confPeerListChanged (confNum);
				}> ();

		// groups
		Register<tox_callback_group_peer_join,
				[] (ToxW& self, Tox*, uint32_t groupNum, uint32_t peerId)
				{
					emit self.Runner_.groupPeerJoined (groupNum, peerId);
				}> ();
		Register<tox_callback_group_peer_exit,
				[] (ToxW& self, Tox*,
						uint32_t groupNum, uint32_t peerId,
						Tox_Group_Exit_Type exitType,
						const uint8_t *namePtr, size_t nameLen,
						const uint8_t *partMsgPtr, size_t partMsgLen)
				{
					emit self.Runner_.groupPeerExited (groupNum, peerId,
							{
								.Type_ = FromToxEnum (exitType),
								.Nick_ = FromToxStr (namePtr, nameLen),
								.PartMsg_ = FromToxStr (partMsgPtr, partMsgLen)
							});
				}> ();
	}

	Util::Either<ToxError<InitError>, Util::Void> ToxW::Init (EntryStatus initialStatus)
	{
		emit Runner_.statusChanged ({ SConnecting, {} });

		const Tox_Options opts
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
			.savedata_data = std::bit_cast<const uint8_t*> (ToxState_.constData ()),
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

		TOX_ERR_NEW initError {};
		Tox_ = std::unique_ptr<Tox, void (*) (Tox*)> (tox_new (&opts, &initError), &tox_kill);
		if (!Tox_ || initError != TOX_ERR_NEW_OK)
			co_return Util::Left { MapError (initError) };

		//CallManager_ = std::make_shared<CallManager> (this, Tox_.get ());

		if (const auto setNameRes = WithStrError (&tox_self_set_name, Tox_.get (), Name_);
			setNameRes.IsLeft ())
			qWarning () << "unable to set self name" << setNameRes.GetLeft ();

		InitializeCallbacks ();

		qDebug () << "gonna bootstrap..." << Tox_.get ();
		const auto pubkey = Hex2Bin ("F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67"_qba);

		co_await WithError (&tox_bootstrap,
				Tox_.get (),
				"192.210.149.121",
				static_cast<uint16_t> (33445),
				std::bit_cast<const uint8_t*> (pubkey.constData ()));

		SetStatus (initialStatus);

		QTimer::singleShot (0, this, &ToxW::Iterate);

		co_return Util::Void {};
	}

	void ToxW::Iterate ()
	{
		tox_iterate (Tox_.get (), this);
		const auto next = tox_iteration_interval (Tox_.get ());
		QTimer::singleShot (std::chrono::milliseconds { next }, this, &ToxW::Iterate);
	}
}
