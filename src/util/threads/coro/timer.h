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

namespace LC::Util
{
	namespace detail
	{
		struct TimerAwaiter
		{
			std::chrono::milliseconds Duration_;
			Qt::TimerType Precision_;

			bool await_ready () const noexcept { return false; }

			void await_suspend (std::coroutine_handle<> handle) noexcept
			{
				QTimer::singleShot (Duration_, handle);
			}

			void await_resume () const noexcept {}
		};
	}

	template<typename Rep, typename Period>
	auto operator co_await (std::chrono::duration<Rep, Period> duration)
	{
		return detail::TimerAwaiter { duration };
	}

	template<Qt::TimerType Precision>
	struct WithPrecision
	{
		std::chrono::milliseconds Duration_;

		auto operator co_await () const
		{
			return detail::TimerAwaiter { Duration_, Precision };
		}
	};

	using Precise = WithPrecision<Qt::PreciseTimer>;
	using Coarse = WithPrecision<Qt::CoarseTimer>;
	using VeryCoarse = WithPrecision<Qt::VeryCoarseTimer>;
}
