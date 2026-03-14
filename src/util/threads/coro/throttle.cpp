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
					if (!Queue_.isEmpty ())
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

	Throttle::Awaiter::Awaiter (Throttle& throttle)
	: Throttle_ { throttle }
	{
	}

	Throttle::Awaiter::Awaiter (Awaiter&& other) noexcept
	: Throttle_ { other.Throttle_ }
	, Handle_ { std::exchange (other.Handle_, {}) }
	{
	}

	Throttle::Awaiter::~Awaiter ()
	{
		if (Handle_)
			Throttle_.Queue_.removeOne (Handle_);
	}

	bool Throttle::Awaiter::await_ready () const
	{
		const bool allowed = std::chrono::milliseconds { Throttle_.LastInvocation_.elapsed () } >= Throttle_.Interval_ && Throttle_.Queue_.isEmpty ();
		if (allowed)
			Throttle_.LastInvocation_.restart ();
		return allowed;
	}

	void Throttle::Awaiter::await_suspend (std::coroutine_handle<> handle)
	{
		if (Throttle_.Queue_.isEmpty ())
			Throttle_.StartTimer (Throttle_.Interval_ - std::chrono::milliseconds { Throttle_.LastInvocation_.elapsed () });

		Throttle_.Queue_ << handle;
		Handle_ = handle;
	}

	void Throttle::Awaiter::await_resume ()
	{
		Handle_ = {};
	}

	Throttle::Awaiter Throttle::operator co_await ()
	{
		return Awaiter { *this };
	}

	void Throttle::StartTimer (std::chrono::milliseconds timeout)
	{
		BackoffFactor_ = std::max (0, BackoffFactor_ - 1);
		Timer_.start (timeout * (BackoffFactor_ + 1));
	}
}
