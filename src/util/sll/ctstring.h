/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <algorithm>

class QByteArray;

namespace LC::Util
{
	template<size_t N>
	using RawStr = const char (&) [N];

	template<size_t N>
	struct CtString
	{
		char Data_ [N] {};

		consteval CtString () = default;

		consteval CtString (RawStr<N> s)
		{
			std::copy (std::begin (s), std::end (s), Data_);
		}

		consteval static auto FromUnsized (const char *s)
		{
			CtString result {};
			std::copy (s, s + N, result.Data_);
			return result;
		}

		template<size_t N2>
		consteval auto operator+ (const CtString<N2>& s2) const
		{
			CtString<N + N2 - 1> result;
			std::copy (std::begin (Data_), std::end (Data_) - 1, result.Data_);
			std::copy (std::begin (s2.Data_), std::end (s2.Data_), result.Data_ + N - 1);
			return result;
		}

		template<size_t N2>
		consteval auto operator+ (RawStr<N2> s2) const
		{
			return *this + CtString<N2> { static_cast<RawStr<N2>> (s2) };
		}

		constexpr RawStr<N> GetRawSized () const
		{
			return Data_;
		}

		QByteArray ToByteArray () const
		{
			// TODO hack around QByteArrayLiteral
			return QByteArray { Data_ };
		}
	};

	template<size_t N1, size_t N2>
	consteval auto operator+ (RawStr<N1> s1, CtString<N2> s2)
	{
		return CtString<N1> { s1 } + s2;
	}

	consteval size_t StringBufSize (const char *str)
	{
		size_t result = 0;
		while (str [result++])
			;
		return result;
	}

	template<size_t N>
	CtString (RawStr<N>) -> CtString<N>;
}

namespace LC
{
	template<Util::CtString S>
	consteval auto operator""_ct ()
	{
		return S;
	}
}
