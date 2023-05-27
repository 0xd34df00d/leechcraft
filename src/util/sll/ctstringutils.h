/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ctstring.h"
#include <QDebug>

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
		requires (Tup1Size == Tup2Size)
	constexpr auto ZipWith (Tup1&& tup1, auto&& sep, Tup2&& tup2) noexcept
	{
		return [&]<size_t... I> (std::index_sequence<I...>)
		{
			return std::tuple { (std::get<I> (tup1) + sep + std::get<I> (tup2))... };
		} (std::make_index_sequence<Tup1Size> {});
	}

	namespace detail
	{
		template<typename T1, typename T2>
		consteval bool JMEq (const T1& v1, const T2& v2)
		{
			if constexpr (!std::is_same_v<T1, T2>)
				return false;
			else
				return v1 == v2;
		}
	}

	template<const auto& F>
	constexpr auto Nub ()
	{
		constexpr auto tup = F ();
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<decltype (tup)>> {};

		return [&]<std::size_t... Ix> (std::index_sequence<Ix...>)
		{
			return std::tuple_cat ([&]
				{
					constexpr auto thisIndex = Ix;
					constexpr auto item = std::get<thisIndex> (tup);

					constexpr auto itemResult = [&]<std::size_t... IxOther> (std::index_sequence<IxOther...>)
					{
						if constexpr (((detail::JMEq (item, std::get<IxOther> (tup)) && IxOther < thisIndex) || ...))
							return std::tuple {};
						else
							return std::tuple { item };
					} (indices);
					return itemResult;
				} ()...);
		} (indices);
	}

	template<size_t N, typename Char>
	QDebug operator<< (QDebug dbg, const CtString<N, Char>& str)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "CtString[" << N << "] { ";
		for (size_t i = 0; i < N; ++i)
			dbg.nospace () << str.Data_ [i];
		dbg.nospace () << " }";
		return dbg;
	}
}
