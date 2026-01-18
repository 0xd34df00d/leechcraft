/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstdint>
#include <QDebug>
#include <util/sll/either.h>

namespace LC::Azoth::Sarin
{
	template<typename T>
		requires (std::is_enum_v<T>)
	struct ToxError
	{
		T Code_;
		QString Message_;

		friend QDebug operator<< (QDebug out, const ToxError<T>& err)
		{
			const QDebugStateSaver saver { out };
			out.nospace () << static_cast<int> (err.Code_) << ": " << err.Message_;
			return out;
		}
	};

	enum class AddFriendError : std::uint8_t
	{
		InvalidId,
		TooLong,
		NoMessage,
		OwnKey,
		AlreadySent,
		BadChecksum,
		NoSpam,
		NoMem,

		UnknownError
	};

	using AddFriendResult = Util::Either<ToxError<AddFriendError>, uint32_t>;

	enum class FriendQueryError : std::uint8_t
	{
		NotFound,

		UnknownError
	};

	enum class InitError : std::uint8_t
	{
		NullParam,
		NoMem,
		PortAlloc,
		BadProxyType,
		BadProxyHost,
		BadProxyPort,
		ProxyNotFound,
		StateEncrypted,
		BadLoadFormat,

		BadBootstrapHost,
		BadBootstrapPort,

		UnknownError
	};

	enum class SetInfoError : std::uint8_t
	{
		NullParam,
		TooLong,

		UnknownError
	};

	enum class FileControlError : std::uint8_t
	{
		FriendNotFound,
		FriendNotConnected,
		TransferNotFound,
		NotPaused,
		Denied,
		AlreadyPaused,
		PacketQueueFull,

		UnknownError
	};

	enum class JoinGroupError : std::uint8_t
	{
		InvalidGroupIdLength,
		ToxOffline,

		InitFailure,
		BadChatId,
		EmptyName,
		NameTooLong,
		PasswordFailure,
		CoreFailure,

		UnknownError
	};

	enum class ToxMessageType : std::uint8_t
	{
		Normal,
		Action,
	};

	enum class ConfType : std::uint8_t
	{
		Text,
		AV,
	};

	struct ConfInvitationEvent
	{
		uint32_t FriendNum_;
		ConfType Type_;
		QByteArray Cookie_;
	};

	struct ConfTitleChangedEvent
	{
		uint32_t PeerNum_;
		QString Title_;
	};

	struct ConfPeerNameChangedEvent
	{
		uint32_t PeerNum_;
		QString NewName_;
	};

	struct ConfMessageEvent
	{
		uint32_t PeerNum_;
		ToxMessageType Type_;
		QString Message_;
	};

	enum class GroupExitType : std::uint8_t
	{
		Quit,
		Timeout,
		Disconnected,
		SelfDisconnected,
		Kicked,
		SyncError,
	};

	struct GroupPeerExitedEvent
	{
		GroupExitType Type_;
		QString Nick_;
		QString PartMsg_;
	};
}
