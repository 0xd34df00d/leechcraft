/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <algorithm>
#include <concepts>

class QByteArray;

namespace LC::Util
{
	template<size_t N, typename Char = char>
	using RawStr = const Char (&) [N];

	template<size_t N, typename Char = char>
	struct CtString
	{
		using Char_t = Char;

		constexpr static size_t Size = N - 1;

		Char Data_ [N] {};

		constexpr CtString () noexcept = default;

		constexpr CtString (RawStr<N, Char> s) noexcept
		{
			std::copy (std::begin (s), std::end (s), Data_);
		}

		constexpr static auto FromUnsized (const Char *s) noexcept
		{
			CtString result {};
			std::copy (s, s + N, result.Data_);
			result.Data_ [N - 1] = '\0';
			return result;
		}

		template<size_t N2>
		constexpr auto operator+ (const CtString<N2, Char>& s2) const noexcept
		{
			CtString<Size + s2.Size + 1, Char> result;
			std::copy (Data_, Data_ + Size, result.Data_);
			std::copy (s2.Data_, s2.Data_ + s2.Size + 1, result.Data_ + Size);
			return result;
		}

		template<size_t N2>
		constexpr auto operator+ (RawStr<N2, Char> s2) const noexcept
		{
			return *this + CtString<N2, Char> { static_cast<RawStr<N2, Char>> (s2) };
		}

		constexpr bool IsEmpty () const noexcept
		{
			return !Size;
		}

		constexpr RawStr<N, Char> GetRawSized () const noexcept
		{
			return Data_;
		}

		constexpr bool EndsWith (Char ch) const noexcept
			requires (Size > 0)
		{
			return Data_ [Size - 1] == ch;
		}

		template<size_t Count>
			requires (Count < N)
		constexpr auto Chop () const noexcept
		{
			return CtString<N - Count, Char>::FromUnsized (Data_);
		}

		QByteArray ToByteArray () const noexcept
			requires std::same_as<Char, char>
		{
			// TODO hack around QByteArrayLiteral
			return QByteArray { Data_ };
		}

		QString ToString () const noexcept
		{
			// TODO do this at compile-time
			return ToByteArray ();
		}
	};

	template<size_t N1, size_t N2, typename Char>
	constexpr auto operator+ (RawStr<N1, Char> s1, CtString<N2, Char> s2) noexcept
	{
		return CtString<N1, Char> { s1 } + s2;
	}

	template<typename Char>
	constexpr size_t StringBufSize (const Char *str) noexcept
	{
		size_t result = 0;
		while (str [result++])
			;
		return result;
	}

	template<size_t N, typename Char>
	CtString (RawStr<N, Char>) -> CtString<N, Char>;
}

namespace LC
{
	template<Util::CtString S>
	constexpr auto operator""_ct ()
	{
		return S;
	}
}

namespace LC::Util
{
	constexpr auto Join (auto&&) noexcept
	{
		return ""_ct;
	}

	constexpr auto Join (auto&& sep, auto&& first, auto&&... strings) noexcept
	{
		if constexpr (sizeof... (strings))
			return first + ((sep + strings) + ...);
		else
			return first;
	}

	constexpr auto JoinTup (auto&& stringsTuple, auto&& sep) noexcept
	{
		return std::apply ([&sep]<typename... Ts> (Ts&&... args) { return Join (sep, std::forward<Ts> (args)...); },
				stringsTuple);
	}

	template<typename Tup1, typename Tup2,
			size_t Tup1Size = std::tuple_size_v<std::decay_t<Tup1>>,
			size_t Tup2Size = std::tuple_size_v<std::decay_t<Tup2>>
		>
	constexpr auto ZipWith (Tup1&& tup1, auto&& sep, Tup2&& tup2) noexcept
		requires (Tup1Size == Tup2Size)
	{
		return [&]<size_t... I> (std::index_sequence<I...>)
		{
			return std::tuple { (std::get<I> (tup1) + sep + std::get<I> (tup2))... };
		} (std::make_index_sequence<Tup1Size> {});
	}
}
