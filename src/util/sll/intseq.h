/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>

namespace LC
{
namespace Util
{
namespace IntSeq
{
	template<typename T, T... Fst, T... Snd>
	std::integer_sequence<T, Fst..., Snd...>
	ConcatImpl (std::integer_sequence<T, Fst...>, std::integer_sequence<T, Snd...>);

	template<typename... Seqs>
	struct ConcatS;

	template<typename... Seqs>
	using Concat = typename ConcatS<Seqs...>::Type_t;

	template<typename Seq>
	struct ConcatS<Seq>
	{
		using Type_t = Seq;
	};

	template<typename Seq1, typename Seq2, typename... Rest>
	struct ConcatS<Seq1, Seq2, Rest...>
	{
		using Type_t = Concat<decltype (ConcatImpl (Seq1 {}, Seq2 {})), Rest...>;
	};

	template<typename T, T E, size_t C>
	struct RepeatS
	{
		template<T... Is>
		static auto RepeatImpl (std::integer_sequence<T, Is...>)
		{
			return std::integer_sequence<T, (static_cast<void> (Is), E)...> {};
		}

		using Type_t = decltype (RepeatImpl (std::make_integer_sequence<T, C> {}));
	};

	template<typename T, T E, size_t C>
	using Repeat = typename RepeatS<T, E, C>::Type_t;
}
}
}
