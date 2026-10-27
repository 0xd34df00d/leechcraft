/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "timer.h"

namespace LC::Util::detail
{
	TimerAwaiter::TimerAwaiter (std::chrono::milliseconds duration, Qt::TimerType precision)
	{
		Timer_.setSingleShot (true);
		Timer_.setInterval (duration);
		Timer_.setTimerType (precision);
	}

	bool TimerAwaiter::await_ready () const noexcept
	{
		return false;
	}

	void TimerAwaiter::await_suspend (std::coroutine_handle<> handle) noexcept
	{
		Timer_.callOnTimeout (&Timer_, handle);
		Timer_.start ();
	}

	void TimerAwaiter::await_resume () const noexcept
	{
	}
}
