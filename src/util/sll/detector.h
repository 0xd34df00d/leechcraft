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
	namespace detail
	{
		template<typename Default, typename Placeholder, template<typename...> class Op, typename... Args>
		struct IsDetected
		{
			using value_t = std::false_type;
			using type = Default;
		};

		template<typename Default, template<typename...> class Op, typename... Args>
		struct IsDetected<Default, std::void_t<Op<Args...>>, Op, Args...>
		{
			using value_t = std::true_type;
			using type = Op<Args...>;
		};
	}

	template<template<typename...> class Op, typename... Args>
	constexpr bool IsDetected_v = detail::IsDetected<void, void, Op, Args...>::value_t::value;

	template<typename Type, template<typename...> class Op, typename... Args>
	using IsDetected_t = typename detail::IsDetected<Type, void, Op, Args...>::type;
}
}
