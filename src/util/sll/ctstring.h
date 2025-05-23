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
#include <QString>

class QByteArray;

namespace LC::Util
{
	template<size_t N, typename Char = char>
	using RawStr = const Char (&) [N];

	/** Non-0-terminated compile-time string.
	 *
	 * @tparam N The length of the string in `Char`.
	 * @tparam Char The underlying character type.
	 */
	template<size_t N, typename Char = char>
	struct CtString
	{
		using Char_t = Char;

		/** The size of the string.
		 */
		constexpr static size_t Size = N;

		Char Data_ [Size] {};

		constexpr CtString () noexcept = default;

		constexpr CtString (RawStr<N + 1, Char> s) noexcept
		{
			std::copy (s, s + Size, Data_);
		}

		constexpr auto operator<=> (const CtString&) const = default;

		constexpr static auto FromUnsized (const Char *s) noexcept
		{
			CtString result {};
			std::copy (s, s + Size, result.Data_);
			return result;
		}

		template<size_t N2>
		constexpr auto operator+ (const CtString<N2, Char>& s2) const noexcept
		{
			// TODO clang bug, use s2.Size otherwise
			CtString<Size + CtString<N2, Char>::Size, Char> result;
			std::copy (Data_, Data_ + Size, result.Data_);
			std::copy (s2.Data_, s2.Data_ + s2.Size, result.Data_ + Size);
			return result;
		}

		template<size_t N2>
		constexpr auto operator+ (RawStr<N2, Char> s2) const noexcept
		{
			return *this + CtString<N2 - 1, Char> { s2 };
		}

		constexpr auto operator+ (Char ch) const noexcept
		{
			return *this + CtString<1, Char> { { ch } };
		}

		constexpr bool IsEmpty () const noexcept
		{
			return !Size;
		}

		constexpr bool EndsWith (Char ch) const noexcept
			requires (Size > 0)
		{
			return Data_ [Size - 1] == ch;
		}

		template<size_t Count>
			requires (Count <= Size)
		[[nodiscard]] constexpr auto Chop () const noexcept
		{
			return CtString<N - Count, Char>::FromUnsized (Data_);
		}

		constexpr Char& operator[] (size_t pos) noexcept
		{
			return Data_ [pos];
		}

		constexpr Char operator[] (size_t pos) const noexcept
		{
			return Data_ [pos];
		}

		constexpr operator QStringView () const noexcept
			requires std::is_same_v<Char, char16_t>
		{
			return QStringView { Data_, Size };
		}

		constexpr auto Data () const noexcept
		{
			return Data_;
		}

		template<typename NewChar>
		constexpr CtString<N, NewChar> CastChars () const noexcept
		{
			CtString<N, NewChar> result;
			std::copy (Data_, Data_ + N, result.Data_);
			return result;
		}
	};

	template<CtString Str>
	QByteArray ToByteArray ()
	{
		constexpr static auto terminated = Str + '\0';
		// this const_cast is fine-ish, since Qt is doing the same in QByteArrayLiteral()
		return QByteArray { QByteArrayData { nullptr, const_cast<char*> (terminated.Data_), terminated.Size - 1 } };
	}

	template<CtString Str>
	QString ToString ()
	{
		if constexpr (std::is_same_v<typename decltype (Str)::Char_t, char16_t>)
		{
			constexpr static auto terminated = Str + '\0';
			// this const_cast is fine-ish, since Qt is doing the same in QtPrivate::qMakeStringPrivate()
			return QString { QStringPrivate { nullptr, const_cast<char16_t*> (terminated.Data_), terminated.Size - 1 } };
		}
		else
			return ToString<Str.template CastChars<char16_t> ()> ();
	}

	template<size_t N1, size_t N2, typename Char>
	constexpr auto operator+ (RawStr<N1, Char> s1, CtString<N2, Char> s2) noexcept
	{
		return CtString<N1 - 1, Char> { s1 } + s2;
	}

	template<typename Char>
	constexpr size_t StringBufSize (const Char *str) noexcept
	{
		size_t result = 0;
		while (str [result++])
			;
		return result - 1;
	}

	template<size_t N, typename Char>
	CtString (RawStr<N, Char>) -> CtString<N - 1, Char>;
}

namespace LC
{
	template<Util::CtString S>
	constexpr auto operator""_ct () noexcept
	{
		return S;
	}
}
