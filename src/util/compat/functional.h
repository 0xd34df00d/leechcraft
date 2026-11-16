/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>

namespace LC::Util::detail
{
	template<typename>
	struct StdFunctionFor;

	template<typename R, typename... Args>
	struct StdFunctionFor<R (Args...)>
	{
		using Type = std::function<R (Args...)>;
	};

	template<typename R, typename... Args>
	struct StdFunctionFor<R (Args...) const>
	{
		using Type = std::function<R (Args...)>;
	};

	template<typename R, typename... Args>
	struct StdFunctionFor<R (Args...) noexcept>
	{
		using Type = std::function<R (Args...)>;
	};

	template<typename R, typename... Args>
	struct StdFunctionFor<R (Args...) const noexcept>
	{
		using Type = std::function<R (Args...)>;
	};
}

// https://github.com/microsoft/STL/issues/3803
// https://github.com/llvm/llvm-project/issues/105387
#ifndef __cpp_lib_copyable_function
namespace std
{
	template<typename Signature>
	using copyable_function = typename LC::Util::detail::StdFunctionFor<Signature>::Type;
}
#endif

// https://github.com/llvm/llvm-project/issues/105157
#ifndef __cpp_lib_move_only_function
namespace std
{
	template<typename Signature>
	using move_only_function = typename LC::Util::detail::StdFunctionFor<Signature>::Type;
}
#endif
