/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <array>
#include <optional>
#include <QString>
#include <tox/tox.h>
#include <util/sll/typegetter.h>
#include <util/sll/void.h>
#include <util/threads/coro/eithercoro.h>
#include <interfaces/azoth/imucentry.h>
#include "types.h"

namespace LC::Azoth
{
	class ICLEntry;
}

namespace LC::Azoth::Sarin
{
	class ToxAccount;

	std::optional<uint32_t> GetFriendId (const Tox *tox, Pubkey pubkey);
	std::optional<Pubkey> GetFriendPubkey (const Tox *tox, uint32_t friendId);

	QString ToxId2HR (const uint8_t *address, size_t Size);

	template<size_t Size>
	QString ToxId2HR (const std::array<uint8_t, Size>& address)
	{
		return ToxId2HR (address.data (), Size);
	}

	template<size_t Size>
	QDebug operator<< (QDebug out, const std::array<uint8_t, Size>& id)
	{
		return out << ToxId2HR (id);
	}

	template<size_t Size>
	std::optional<std::array<uint8_t, Size>> HumanReadable2ToxId (const QByteArray& hex)
	{
		const auto& bin = QByteArray::fromHex (hex);
		if (bin.size () != Size)
			return {};

		std::array<uint8_t, Size> result;
		std::copy_n (bin.begin (), Size, result.begin ());
		return result;
	}

	template<size_t Size>
	std::optional<std::array<uint8_t, Size>> HumanReadable2ToxId (const QString& hex)
	{
		return HumanReadable2ToxId<Size> (hex.toUtf8 ());
	}

	QString FromToxStr (const uint8_t *data, size_t size);
	QByteArray FromToxBytes (const uint8_t *data, qsizetype size);

	ConfType FromToxEnum (TOX_CONFERENCE_TYPE);
	GroupExitType FromToxEnum (Tox_Group_Exit_Type);
	ToxMessageType FromToxEnum (TOX_MESSAGE_TYPE);

	QString ToUserString (AddFriendError);
	QString ToUserString (FriendQueryError);
	QString ToUserString (Tox_Err_Conference_Invite);
	QString ToUserString (Tox_Err_Group_Invite_Friend);

	void ReportError (const QString& text);

	std::optional<Pubkey> GetEntryPubkey (const ICLEntry& entry);
	IMUCEntry::InviteCheckResult_t CheckInvitee (const ToxAccount& account, const ICLEntry& entry);

	InitError MapErrorCode (TOX_ERR_BOOTSTRAP error);
	FileControlError MapErrorCode (TOX_ERR_FILE_CONTROL error);
	AddFriendError MapErrorCode (TOX_ERR_FRIEND_ADD error);
	FriendQueryError MapErrorCode (TOX_ERR_FRIEND_QUERY error);
	JoinGroupError MapErrorCode (Tox_Err_Group_Join error);
	InitError MapErrorCode (TOX_ERR_NEW error);
	SetInfoError MapErrorCode (TOX_ERR_SET_INFO error);

	template<typename EC>
	constexpr inline auto MapErrorMessage = Util::Void {};

	template<> constexpr inline auto MapErrorMessage<TOX_ERR_BOOTSTRAP> = &tox_err_bootstrap_to_string;
	template<> constexpr inline auto MapErrorMessage<TOX_ERR_FILE_CONTROL> = &tox_err_file_control_to_string;
	template<> constexpr inline auto MapErrorMessage<TOX_ERR_FRIEND_ADD> = &tox_err_friend_add_to_string;
	template<> constexpr inline auto MapErrorMessage<TOX_ERR_FRIEND_QUERY> = &tox_err_friend_query_to_string;
	template<> constexpr inline auto MapErrorMessage<TOX_ERR_NEW> = &tox_err_new_to_string;
	template<> constexpr inline auto MapErrorMessage<TOX_ERR_SET_INFO> = &tox_err_set_info_to_string;

	template<typename EC>
	concept KnownErrorCode = requires (EC ec) { MapErrorCode (ec); };

	template<typename EC>
	concept KnownErrorMessage = KnownErrorCode<EC> && !std::is_same_v<std::decay_t<decltype (MapErrorMessage<EC>)>, Util::Void>;

	template<KnownErrorMessage EC>
	ToxError<decltype (MapErrorCode (EC {}))> MapError (EC error)
	{
		const auto& msg = QString::fromUtf8 (MapErrorMessage<EC> (error));
		return { MapErrorCode (error), msg };
	}

	template<KnownErrorCode EC>
	auto MapError (EC error)
	{
		return MapErrorCode (error);
	}

	auto MapError (auto error)
	{
		return error;
	}

	template<typename F>
	using ErrorType_t = std::remove_pointer_t<Util::ArgType_t<F, Util::ArgCount_v<F> - 1>>;

	template<typename F>
	using MappedErrorType_t = decltype (MapError (ErrorType_t<F> {}));

	template<typename F>
	using EitherType_t = Util::Either<MappedErrorType_t<F>, Util::RetType_t<F>>;

	template<typename F>
	EitherType_t<F> WithError (F f, auto... args)
	{
		ErrorType_t<F> error {};
		auto result = f (args..., &error);
		if (error)
		{
			const auto mapped = MapError (error);
			if constexpr (KnownErrorMessage<ErrorType_t<F>>)
				qWarning () << mapped;
			else
				qWarning () << error;
			return { Util::AsLeft, mapped };
		}
		return result;
	}

	namespace detail
	{
		inline auto Unwrap (const QByteArray& bytes)
		{
			return std::tuple { std::bit_cast<const uint8_t*> (bytes.constData ()), bytes.size () };
		}

		template<size_t N>
		auto Unwrap (const std::array<uint8_t, N>& data)
		{
			return std::tuple { data.data () };
		}

		auto Unwrap (const QString& str) = delete ("Use `toUtf8()` or `toLatin1()` and pass the QByteArray");

		template<typename T>
		auto Unwrap (T t)
		{
			return std::tuple { t };
		}
	}

	template<typename F, typename... Args>
		requires (!std::is_pointer_v<Args> && ...) &&
				(!std::is_same_v<Args, QString> && ...)
	auto WithUnwrapping (F f, Tox *tox, Args... args)
	{
		return std::apply ([&] (auto... args) { return f (tox, args...); }, std::tuple_cat (detail::Unwrap (args)...));
	}

	template<typename F, typename... Args>
	EitherType_t<F> WithErrorUnwrapping (F f, Tox *tox, Args... args)
	{
		return WithUnwrapping ([f]<typename... Ts> (Ts&&... subargs) { return WithError (f, std::forward<Ts> (subargs)...); }, tox, args...);
	}

	template<typename SizeF, typename CharsF, typename... Args>
		requires (std::is_same_v<MappedErrorType_t<SizeF>, MappedErrorType_t<CharsF>>)
	Util::Either<MappedErrorType_t<SizeF>, QString> QueryToxString (SizeF sizeF, CharsF charsF, Args... args)
	{
		const auto size = co_await WithError (sizeF, args...);
		const auto bytes = std::make_unique<uint8_t []> (size);
		co_await WithError (charsF, args..., &bytes [0]);
		co_return FromToxStr (&bytes [0], size);
	}

	template<size_t Size, typename F, typename... Args>
	Util::Either<MappedErrorType_t<F>, std::array<uint8_t, Size>> QueryToxBytes (F query, Args... args)
	{
		std::array<uint8_t, Size> bytes {};
		co_await WithError (query, args..., &bytes [0]);
		co_return bytes;
	}
}
