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
	};

	template<CtString Str>
	QByteArray ToByteArray ()
	{
		static constexpr auto literal = []<size_t... Idxes> (std::index_sequence<Idxes...>)
		{
			return QStaticByteArrayData<Str.Size>
			{
				Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER (Str.Size),
				{ Str.Data_ [Idxes]..., 0 }
			};
		} (std::make_index_sequence<Str.Size> {});
		QByteArrayDataPtr holder { literal.data_ptr () };
		return QByteArray { holder };
	}

	template<CtString Str>
	QString ToString ()
	{
		static constexpr auto literal = []<size_t... Idxes> (std::index_sequence<Idxes...>)
		{
			return QStaticStringData<Str.Size>
			{
				Q_STATIC_STRING_DATA_HEADER_INITIALIZER (Str.Size),
				{ Str.Data_ [Idxes]..., 0 }
			};
		} (std::make_index_sequence<Str.Size> {});
		QStringDataPtr holder { literal.data_ptr () };
		return QString { holder };
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
