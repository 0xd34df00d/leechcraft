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
		Char Data_ [N] {};

		consteval CtString () = default;

		consteval CtString (RawStr<N, Char> s)
		{
			std::copy (std::begin (s), std::end (s), Data_);
		}

		consteval static auto FromUnsized (const Char *s)
		{
			CtString result {};
			std::copy (s, s + N, result.Data_);
			return result;
		}

		template<size_t N2>
		consteval auto operator+ (const CtString<N2, Char>& s2) const
		{
			CtString<N + N2 - 1, Char> result;
			std::copy (std::begin (Data_), std::end (Data_) - 1, result.Data_);
			std::copy (std::begin (s2.Data_), std::end (s2.Data_), result.Data_ + N - 1);
			return result;
		}

		template<size_t N2>
		consteval auto operator+ (RawStr<N2, Char> s2) const
		{
			return *this + CtString<N2, Char> { static_cast<RawStr<N2, Char>> (s2) };
		}

		constexpr RawStr<N, Char> GetRawSized () const
		{
			return Data_;
		}

		QByteArray ToByteArray () const
			requires std::same_as<Char, char>
		{
			// TODO hack around QByteArrayLiteral
			return QByteArray { Data_ };
		}
	};

	template<size_t N1, size_t N2, typename Char>
	consteval auto operator+ (RawStr<N1, Char> s1, CtString<N2, Char> s2)
	{
		return CtString<N1, Char> { s1 } + s2;
	}

	template<typename Char>
	consteval size_t StringBufSize (const Char *str)
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
	consteval auto operator""_ct ()
	{
		return S;
	}
}
