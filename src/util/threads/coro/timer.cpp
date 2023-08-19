/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "timer.h"
#include <QTimer>

namespace LC::Util::detail
{
	bool TimerAwaiter::await_ready () const noexcept
	{
		return false;
	}

	void TimerAwaiter::await_suspend (std::coroutine_handle<> handle) noexcept
	{
		QTimer::singleShot (Duration_, handle);
	}

	void TimerAwaiter::await_resume () const noexcept
	{
	}
}

namespace LC::Util
{
	detail::TimerAwaiter operator co_await (std::chrono::milliseconds duration)
	{
		return detail::TimerAwaiter { duration, Qt::CoarseTimer };
	}
}
