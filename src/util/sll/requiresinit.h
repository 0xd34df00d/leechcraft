/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC::Util
{
	template<typename T>
	struct RequiresInit
	{
		T Val_;

		RequiresInit () = delete;

		constexpr RequiresInit (const RequiresInit&) = default;
		constexpr RequiresInit (RequiresInit&&) = default;

		constexpr RequiresInit& operator= (const RequiresInit&) = default;
		constexpr RequiresInit& operator= (RequiresInit&&) = default;

		constexpr RequiresInit (T&& val)
		: Val_ { std::forward<T> (val) }
		{
		}

		template<typename... Args>
		constexpr RequiresInit (Args&&... args)
		: Val_ { std::forward<Args> (args)... }
		{
		}

		constexpr operator T () const
		{
			return Val_;
		}
	};
}
