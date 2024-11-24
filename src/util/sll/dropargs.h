/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "typelist.h"

namespace LC
{
namespace Util
{
	namespace detail
	{
		template<typename F, template<typename...> class List, typename... Args>
		constexpr List<Args...> GetInvokablePartImpl (int, List<Args...>, typename std::result_of<F (Args...)>::type* = nullptr)
		{
			return {};
		}

		template<typename F, template<typename...> class List>
		constexpr Typelist<> GetInvokablePartImpl (float, List<>)
		{
			return {};
		}

		template<typename F, typename List>
		struct InvokableType;

		template<typename F, template<typename...> class List, typename... Args>
		constexpr auto GetInvokablePartImpl (float, List<Args...> list) -> typename InvokableType<F, decltype (Reverse (Tail (Reverse (list))))>::RetType_t
		{
			return {};
		}

		template<typename F, typename List>
		struct InvokableType
		{
			using RetType_t = decltype (GetInvokablePartImpl<F> (0, List {}));
		};

		template<typename F, typename... Args>
		constexpr auto GetInvokablePart () -> decltype (GetInvokablePartImpl<F> (0, Typelist<Args...> {}))
		{
			return {};
		}

		template<template<typename...> class List, typename... Args>
		constexpr size_t Length (List<Args...>)
		{
			return sizeof... (Args);
		}

		template<typename T>
		struct Dumbifier
		{
			using Type_t = T;
		};

		template<typename T>
		using Dumbify = typename Dumbifier<T>::Type_t;

		template<typename F, typename List>
		struct InvokableResGetter;

		template<typename F, template<typename...> class List, typename... Args>
		struct InvokableResGetter<F, List<Args...>>
		{
			using RetType_t = std::result_of_t<F (Args...)>;
		};

		template<typename F>
		class Dropper
		{
			F F_;
		public:
			Dropper (const F& f)
			: F_ (f)
			{
			}

			template<typename... Args>
			auto operator() (Args... args)
			{
				auto invokableList = GetInvokablePart<F, Args...> ();
				auto ignoreList = Drop<Length (decltype (invokableList) {})> (Typelist<Args...> {});
				return Invoke (invokableList, ignoreList, args...);
			}
		private:
			template<typename... InvokableArgs, typename... Rest>
			auto Invoke (Typelist<InvokableArgs...>, Typelist<Rest...>, Dumbify<InvokableArgs>... args, Dumbify<Rest>...)
			{
				return F_ (std::forward<InvokableArgs> (args)...);
			}
		};
	}

	template<typename F>
	detail::Dropper<F> DropArgs (const F& f)
	{
		return detail::Dropper<F> { f };
	}
}
}
