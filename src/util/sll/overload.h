/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace Util
{
	template<typename... Args>
	struct OverloadBase
	{
		template<typename R, typename C>
		constexpr auto operator() (R (C::*f) (Args...)) const
		{
			return f;
		}

		template<typename R, typename C>
		constexpr auto operator() (R (C::*f) (Args...) const) const
		{
			return f;
		}

		template<typename R>
		constexpr auto operator() (R (*f) (Args...)) const
		{
			return f;
		}
	};

	template<typename... Args>
	constexpr OverloadBase<Args...> Overload {};
}
}
