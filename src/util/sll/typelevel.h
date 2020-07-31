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
	template<template<typename> class Pred>
	struct Not
	{
		template<typename V>
		struct Negate;

		template<bool V>
		struct Negate<std::integral_constant<bool, V>>
		{
			using Result_t = std::integral_constant<bool, !V>;
		};

		template<typename T>
		struct Result_t : Negate<typename Pred<T>::type>::Result_t {};
	};

	template<template<typename> class Pred, typename... Args>
	constexpr auto AllOf = (Pred<Args> {} && ...);

	template<template<typename> class Pred, typename... Args>
	constexpr auto AnyOf = (Pred<Args> {} || ...);
}
}
