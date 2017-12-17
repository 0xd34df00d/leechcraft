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

#include <type_traits>

namespace LeechCraft
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
