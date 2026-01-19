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
#include "types.h"

namespace LC::Azoth::Sarin
{
	std::optional<uint32_t> GetFriendId (const Tox *tox, Pubkey pubkey);
	std::optional<Pubkey> GetFriendPubkey (const Tox *tox, uint32_t friendId);

	template<size_t Size>
	QByteArray ToxId2HR (const uint8_t *address)
	{
		QByteArray result;
		auto toHexChar = [] (uint8_t num) -> char
		{
			return num >= 10 ? (num - 10 + 'A') : (num + '0');
		};

		for (size_t i = 0; i < Size; ++i)
		{
			const auto num = address [i];
			result += toHexChar ((num & 0xf0) >> 4);
			result += toHexChar (num & 0xf);
		}

		return result;
	}

	template<size_t Size>
	QByteArray ToxId2HR (const std::array<uint8_t, Size>& address)
	{
		return ToxId2HR<Size> (address.data ());
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
	auto WithStr (const QString& str, F&& f)
	{
		const auto& strUtf8 = str.toUtf8 ();
		return std::forward<F> (f) (std::bit_cast<const uint8_t*> (strUtf8.constData ()), strUtf8.size ());
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

	template<typename F>
	EitherType_t<F> WithStrError (F f, Tox *tox, const QString& str, auto... args)
	{
		return WithStr (str, [&] (auto bytes, auto size) { return WithError (f, tox, args..., bytes, size); });
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
