/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <chrono>
#include <coroutine>
#include <Qt>
#include "threadsconfig.h"

namespace LC::Util
{
	namespace detail
	{
		struct UTIL_THREADS_API TimerAwaiter
		{
			std::chrono::milliseconds Duration_;
			Qt::TimerType Precision_;

			bool await_ready () const noexcept;
			void await_suspend (std::coroutine_handle<> handle) noexcept;
			void await_resume () const noexcept;
		};
	}

	UTIL_THREADS_API detail::TimerAwaiter operator co_await (std::chrono::milliseconds duration);

	template<Qt::TimerType Precision>
	struct WithPrecision
	{
		std::chrono::milliseconds Duration_;

		auto operator co_await () const
		{
			return detail::TimerAwaiter { Duration_, Precision };
		}
	};

	using Precisely = WithPrecision<Qt::PreciseTimer>;
	using Coarsely = WithPrecision<Qt::CoarseTimer>;
	using VeryCoarsely = WithPrecision<Qt::VeryCoarseTimer>;
}
