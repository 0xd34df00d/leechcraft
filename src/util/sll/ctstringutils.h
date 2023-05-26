/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ctstring.h"

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
}
