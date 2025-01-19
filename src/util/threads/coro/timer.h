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
#include <QTimer>
#include "../threadsconfig.h"

namespace LC::Util
{
	namespace detail
	{
		struct UTIL_THREADS_API TimerAwaiter
		{
			QTimer Timer_;

			explicit TimerAwaiter (std::chrono::milliseconds duration, Qt::TimerType precision);

			bool await_ready () const noexcept;
			void await_suspend (std::coroutine_handle<> handle) noexcept;
			void await_resume () const noexcept;
		};
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

	using Precisely = WithPrecision<Qt::PreciseTimer>;
	using Coarsely = WithPrecision<Qt::CoarseTimer>;
	using VeryCoarsely = WithPrecision<Qt::VeryCoarseTimer>;
}

namespace LC
{
	UTIL_THREADS_API Util::detail::TimerAwaiter operator co_await (std::chrono::milliseconds duration);
}
