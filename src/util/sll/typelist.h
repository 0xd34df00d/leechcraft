/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <tuple>

namespace LeechCraft
{
namespace Util
{
	template<typename...>
	struct Typelist
	{
	};

	template<typename>
	struct Head;

	template<template<typename...> class List, typename H, typename... T>
	struct Head<List<H, T...>>
	{
		using Head_t = H;
	};

	template<typename List>
	using Head_t = typename Head<List>::Head_t;

	template<template<typename...> class List, typename H, typename... T>
	constexpr List<T...> Tail (List<H, T...>)
	{
		return {};
	}

	namespace detail
	{
		template<int N, typename List>
		struct DropImpl
		{
			using Result_t = typename DropImpl<N - 1, decltype (Tail (List {}))>::Result_t;
		};

		template<typename List>
		struct DropImpl<0, List>
		{
			using Result_t = List;
		};
	}

	template<int N, template<typename...> class List, typename... Args>
	constexpr typename detail::DropImpl<N, List<Args...>>::Result_t Drop (List<Args...>)
	{
		return {};
	}

	template<template<typename...> class List, typename... Args1, typename... Args2>
	constexpr List<Args1..., Args2...> Concat (List<Args1...>, List<Args2...>)
	{
		return {};
	}

	template<template<typename...> class List>
	constexpr List<> Reverse (List<>)
	{
		return {};
	}

	template<template<typename...> class List, typename Head, typename... Tail>
	constexpr auto Reverse (List<Head, Tail...>) -> decltype (Concat (Reverse (List<Tail...> {}), List<Head> {}))
	{
		return {};
	}

	namespace detail
	{
		template<template<typename...> class List, typename Tuple, size_t... Is>
		constexpr auto InitImpl (std::integer_sequence<size_t, Is...>)
		{
			return List<std::tuple_element_t<Is, Tuple>...> {};
		}
	}

	template<template<typename...> class List, typename... Args>
	constexpr auto Init (List<Args...>)
	{
		return detail::InitImpl<List, std::tuple<Args...>> (std::make_index_sequence<sizeof... (Args) - 1> {});
	}

	namespace detail
	{
		template<typename Type, template<typename...> class List, typename... Tail>
		constexpr bool HasTypeImpl (List<Type, Tail...>, int)
		{
			return true;
		}

		template<typename, template<typename...> class List>
		constexpr bool HasTypeImpl (List<>, float)
		{
			return false;
		}

		template<typename Type, template<typename...> class List, typename Head, typename... Tail>
		constexpr bool HasTypeImpl (List<Head, Tail...>, float)
		{
			return HasTypeImpl<Type> (List<Tail...> {}, 0);
		}
	}

	template<typename Type, template<typename...> class List, typename... Args>
	constexpr bool HasType (List<Args...> list)
	{
		return detail::HasTypeImpl<Type> (list, 0);
	}

	namespace detail
	{
		template<template<typename> class, typename, typename = void>
		struct Filter;
	}

	template<template<typename> class Pred, typename List>
	using Filter_t = typename detail::Filter<Pred, List>::Result_t;

	namespace detail
	{
		template<template<typename> class Pred, template<typename...> class List, typename Head, typename... Tail>
		struct Filter<Pred, List<Head, Tail...>, std::enable_if_t<Pred<Head>::value>>
		{
			using Result_t = decltype (Concat (List<Head> {}, Filter_t<Pred, List<Tail...>> {}));
		};

		template<template<typename> class Pred, template<typename...> class List, typename Head, typename... Tail>
		struct Filter<Pred, List<Head, Tail...>, std::enable_if_t<!Pred<Head>::value>>
		{
			using Result_t = Filter_t<Pred, List<Tail...>>;
		};

		template<template<typename> class Pred, template<typename...> class List>
		struct Filter<Pred, List<>>
		{
			using Result_t = List<>;
		};
	}

	template<typename T>
	struct AsTypelist;

	template<template<typename...> class OtherList, typename... Args>
	struct AsTypelist<OtherList<Args...>>
	{
		using Result_t = Typelist<Args...>;
	};

	template<typename T>
	using AsTypelist_t = typename AsTypelist<T>::Result_t;

	template<typename F, typename G, typename Def, typename Head, typename... Args>
	auto FirstMatching (F f, G g, Def def, Util::Typelist<Head, Args...>)
	{
		if (f (Head {}))
			return g (Head {});

		if constexpr (sizeof... (Args) > 0)
			return FirstMatching (f, g, def, Util::Typelist<Args...> {});
		else
			return def ();
	}
}
}
