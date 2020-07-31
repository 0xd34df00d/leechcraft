/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>

namespace LC
{
namespace Util
{
	/** @brief Binds an instance of an object to its member function.
	 *
	 * @param[in] fn The member function of class T.
	 * @param[in] c The instance of class T to bind to the member
	 * function \em fn.
	 * @return A functor callable with all arguments of \em fn but the
	 * object itself.
	 * @tparam R The return type of the function.
	 * @tparam T The type of the object.
	 * @tparam Args The arguments to the function, besides the object
	 * itself.
	 */
	template<typename R, typename B, typename C, typename... Args>
	auto BindMemFn (R (B::*fn) (Args...), C *c)
	{
		static_assert (std::is_base_of<B, C> {}, "Base class where the member pointer belongs must be convertible to the binded object's class.");
		return [fn, c] (Args... args) { return (c->*fn) (args...); };
	}

	template<typename R, typename B, typename C, typename... Args>
	auto BindMemFn (R (B::*fn) (Args...) const, const C *c)
	{
		static_assert (std::is_base_of<B, C> {}, "Base class where the member pointer belongs must be convertible to the binded object's class.");
		return [fn, c] (Args... args) { return (c->*fn) (args...); };
	}

	template<typename To>
	struct Caster
	{
		template<typename From>
		std::enable_if_t<!std::is_base_of<To, std::decay_t<From>>::value, To> operator() (From&& from) const
		{
			return To { std::forward<From> (from) };
		}

		template<typename From>
		std::enable_if_t<std::is_base_of<To, std::decay_t<From>>::value, To> operator() (From&& from) const
		{
			return from;
		}
	};

	template<typename To>
	struct Upcaster;

	template<typename To>
	struct Upcaster<To*>
	{
		template<typename From, typename = std::enable_if_t<std::is_base_of_v<To, std::decay_t<From>>>>
		To* operator() (From *from) const
		{
			return from;
		}
	};

	template<typename To>
	constexpr auto Upcast = Upcaster<To> {};
}
}
