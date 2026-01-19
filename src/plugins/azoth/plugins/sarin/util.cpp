/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QtDebug>
#include <util/threads/coro/eithercoro.h>

namespace LC::Azoth::Sarin
{
	std::optional<uint32_t> GetFriendId (const Tox *tox, const QByteArray& pubkey)
	{
		const auto& binPkey = QByteArray::fromHex (pubkey);
		TOX_ERR_FRIEND_BY_PUBLIC_KEY error {};
		const auto res = tox_friend_by_public_key (tox, reinterpret_cast<const uint8_t*> (binPkey.constData ()), &error);
		if (error)
		{
			qWarning () << "failed to get friend by public key" << pubkey << error;
			return {};
		}
		return res;
	}

	std::optional<QByteArray> GetFriendPubkey (const Tox *tox, uint32_t friendId)
	{
		std::array<uint8_t, TOX_PUBLIC_KEY_SIZE> clientId;
		return Util::Visit (WithError (&tox_friend_get_public_key, tox, friendId, clientId.data ()),
				[] (FriendQueryError) { return std::optional<QByteArray> {}; },
				[&] (auto) { return std::optional { ToxId2HR (clientId) }; });
	}

	QString FromToxStr (const uint8_t *data, size_t size)
	{
		return QString::fromUtf8 (std::bit_cast<const char*> (data), size);
	}

	QByteArray FromToxBytes (const uint8_t *data, qsizetype size)
	{
		return QByteArray { std::bit_cast<const char*> (data), size };
	}

	ConfType FromToxEnum (TOX_CONFERENCE_TYPE type)
	{
		using enum ConfType;
		switch (type)
		{
		case TOX_CONFERENCE_TYPE_TEXT:
			return Text;
		case TOX_CONFERENCE_TYPE_AV:
			return AV;
		}

		qWarning () << "unknown conf type" << type;
		return Text;
	}

	GroupExitType FromToxEnum (Tox_Group_Exit_Type type)
	{
		using enum GroupExitType;
		switch (type)
		{
		case TOX_GROUP_EXIT_TYPE_QUIT:
			return Quit;
		case TOX_GROUP_EXIT_TYPE_TIMEOUT:
			return Timeout;
		case TOX_GROUP_EXIT_TYPE_DISCONNECTED:
			return Disconnected;
		case TOX_GROUP_EXIT_TYPE_SELF_DISCONNECTED:
			return SelfDisconnected;
		case TOX_GROUP_EXIT_TYPE_KICK:
			return Kicked;
		case TOX_GROUP_EXIT_TYPE_SYNC_ERROR:
			return SyncError;
		}

		qWarning () << "unknown exit type" << type;
		return Quit;
	}

	ToxMessageType FromToxEnum (TOX_MESSAGE_TYPE type)
	{
		using enum ToxMessageType;
		switch (type)
		{
		case TOX_MESSAGE_TYPE_NORMAL:
			return Normal;
		case TOX_MESSAGE_TYPE_ACTION:
			return Action;
		}

		qWarning () << "unknown message type" << type;
		return Normal;
	}

	namespace
	{
		struct Errors
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Sarin::Errors)
		};
	}

	QString ToUserString (AddFriendError error)
	{
		using enum AddFriendError;
		switch (error)
		{
		case InvalidId:
			return Errors::tr ("invalid Tox ID");
		case TooLong:
			return Errors::tr ("too long greeting message");
		case NoMessage:
			return Errors::tr ("no message");
		case OwnKey:
			return Errors::tr ("why would you add yourself as friend");
		case AlreadySent:
			return Errors::tr ("friend request has already been sent");
		case BadChecksum:
			return Errors::tr ("bad Tox ID checksum");
		case NoSpam:
			return Errors::tr ("nospam value has been changed (get a newer ID!)");
		case NoMem:
			return Errors::tr ("unable to allocate enough memory");
		case UnknownError:
			return Errors::tr ("unknown error");
		}

		std::unreachable ();
	}

	QString ToUserString (FriendQueryError error)
	{
		using enum FriendQueryError;
		switch (error)
		{
		case NotFound:
			return Errors::tr ("contact not found");
		case UnknownError:
			return Errors::tr ("unknown error");
		}

		std::unreachable ();
	}

	AddFriendError MapErrorCode (TOX_ERR_FRIEND_ADD error)
	{
		using enum AddFriendError;
		switch (error)
		{
		case TOX_ERR_FRIEND_ADD_TOO_LONG:
			return TooLong;
		case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
			return NoMessage;
		case TOX_ERR_FRIEND_ADD_OWN_KEY:
			return OwnKey;
		case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
			return AlreadySent;
		case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
			return BadChecksum;
		case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
			return NoSpam;
		case TOX_ERR_FRIEND_ADD_MALLOC:
			return NoMem;
		default:
			return UnknownError;
		}
	}

	FriendQueryError MapErrorCode (TOX_ERR_FRIEND_QUERY error)
	{
		using enum FriendQueryError;
		switch (error)
		{
		case TOX_ERR_FRIEND_QUERY_FRIEND_NOT_FOUND:
			return NotFound;
		default:
			return UnknownError;
		}
	}

	JoinGroupError MapErrorCode (Tox_Err_Group_Join error)
	{
		using enum JoinGroupError;
		switch (error)
		{
		case TOX_ERR_GROUP_JOIN_OK:
			return UnknownError;
		case TOX_ERR_GROUP_JOIN_INIT:
			return InitFailure;
		case TOX_ERR_GROUP_JOIN_BAD_CHAT_ID:
			return BadChatId;
		case TOX_ERR_GROUP_JOIN_EMPTY:
			return EmptyName;
		case TOX_ERR_GROUP_JOIN_TOO_LONG:
			return NameTooLong;
		case TOX_ERR_GROUP_JOIN_PASSWORD:
			return PasswordFailure;
		case TOX_ERR_GROUP_JOIN_CORE:
			return CoreFailure;
		}

		return UnknownError;
	}

	InitError MapErrorCode (TOX_ERR_NEW error)
	{
		using enum InitError;
		switch (error)
		{
		case TOX_ERR_NEW_OK:
			return UnknownError;
		case TOX_ERR_NEW_NULL:
			return NullParam;
		case TOX_ERR_NEW_MALLOC:
			return NoMem;
		case TOX_ERR_NEW_PORT_ALLOC:
			return PortAlloc;
		case TOX_ERR_NEW_PROXY_BAD_TYPE:
			return BadProxyType;
		case TOX_ERR_NEW_PROXY_BAD_HOST:
			return BadProxyHost;
		case TOX_ERR_NEW_PROXY_BAD_PORT:
			return BadProxyPort;
		case TOX_ERR_NEW_PROXY_NOT_FOUND:
			return ProxyNotFound;
		case TOX_ERR_NEW_LOAD_ENCRYPTED:
			return StateEncrypted;
		case TOX_ERR_NEW_LOAD_BAD_FORMAT:
			return BadLoadFormat;
		}

		return UnknownError;
	}

	InitError MapErrorCode (TOX_ERR_BOOTSTRAP error)
	{
		using enum InitError;
		switch (error)
		{
		case TOX_ERR_BOOTSTRAP_OK:
			return UnknownError;
		case TOX_ERR_BOOTSTRAP_NULL:
			return NullParam;
		case TOX_ERR_BOOTSTRAP_BAD_HOST:
			return BadBootstrapHost;
		case TOX_ERR_BOOTSTRAP_BAD_PORT:
			return BadBootstrapHost;
		}

		return UnknownError;
	}

	SetInfoError MapErrorCode (TOX_ERR_SET_INFO error)
	{
		using enum SetInfoError;
		switch (error)
		{
		case TOX_ERR_SET_INFO_OK:
			return UnknownError;
		case TOX_ERR_SET_INFO_NULL:
			return NullParam;
		case TOX_ERR_SET_INFO_TOO_LONG:
			return TooLong;
		}

		return UnknownError;
	}

	FileControlError MapErrorCode (TOX_ERR_FILE_CONTROL error)
	{
		using enum FileControlError;
		switch (error)
		{
		case TOX_ERR_FILE_CONTROL_OK:
			return UnknownError;
		case TOX_ERR_FILE_CONTROL_FRIEND_NOT_FOUND:
			return FriendNotFound;
		case TOX_ERR_FILE_CONTROL_FRIEND_NOT_CONNECTED:
			return FriendNotConnected;
		case TOX_ERR_FILE_CONTROL_NOT_FOUND:
			return TransferNotFound;
		case TOX_ERR_FILE_CONTROL_NOT_PAUSED:
			return NotPaused;
		case TOX_ERR_FILE_CONTROL_DENIED:
			return Denied;
		case TOX_ERR_FILE_CONTROL_ALREADY_PAUSED:
			return AlreadyPaused;
		case TOX_ERR_FILE_CONTROL_SENDQ:
			return PacketQueueFull;
		}

		return UnknownError;
	}
}
