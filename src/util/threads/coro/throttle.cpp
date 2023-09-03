/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "throttle.h"
#include <QTimer>

namespace LC::Util
{
	Throttle::Throttle (std::chrono::milliseconds interval, Qt::TimerType type)
	: Interval_ { interval }
	{
		LastInvocation_.start ();

		Timer_.setTimerType (type);
		Timer_.setSingleShot (true);
		Timer_.callOnTimeout ([this]
				{
					LastInvocation_.restart ();

					if (Queue_.size () > 1)
						StartTimer (Interval_);
					Queue_.takeFirst () ();
				});
	}

	std::chrono::milliseconds Throttle::GetInterval () const
	{
		return Interval_;
	}

	void Throttle::Backoff ()
	{
		BackoffFactor_ += 2;
	}

	bool Throttle::await_ready ()
	{
		const bool allowed = std::chrono::milliseconds { LastInvocation_.elapsed () } >= Interval_ && Queue_.isEmpty ();
		if (allowed)
			LastInvocation_.restart ();
		return allowed;
	}

	void Throttle::await_suspend (std::coroutine_handle<> handle)
	{
		if (Queue_.isEmpty ())
			StartTimer (Interval_ - std::chrono::milliseconds { LastInvocation_.elapsed () });

		Queue_ << handle;
	}

	void Throttle::await_resume () const
	{
	}

	void Throttle::StartTimer (std::chrono::milliseconds timeout)
	{
		BackoffFactor_ = std::max (0, BackoffFactor_ - 1);
		Timer_.start (timeout * (BackoffFactor_ + 1));
	}
}
