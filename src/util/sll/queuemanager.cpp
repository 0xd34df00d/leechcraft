/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "queuemanager.h"
#include <QTimer>

namespace LC
{
namespace Util
{
	QueueManager::QueueManager (int timeout, QObject *parent)
	: QObject (parent)
	, Timeout_ (timeout)
	, ReqTimer_ (new QTimer (this))
	, Paused_ (false)
	{
		ReqTimer_->setSingleShot (true);
		connect (ReqTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (exec ()));
	}

	void QueueManager::Schedule (std::function<void ()> f, QObject *dep, QueuePriority prio)
	{
		const auto& now = QDateTime::currentDateTime ();

		if (prio == QueuePriority::High)
			Queue_.prepend ({ f, dep ? OptionalTracker_t { dep } : OptionalTracker_t () });
		else
			Queue_.append ({ f, dep ? OptionalTracker_t { dep } : OptionalTracker_t () });

		const auto diff = LastRequest_.msecsTo (now);
		if (diff >= Timeout_)
			exec ();
		else if (Queue_.size () == 1)
			ReqTimer_->start (Timeout_ - diff);
	}

	void QueueManager::Clear ()
	{
		Queue_.clear ();
	}

	void QueueManager::Pause ()
	{
		Paused_ = true;
		ReqTimer_->stop ();
	}

	bool QueueManager::IsPaused () const
	{
		return Paused_;
	}

	void QueueManager::Resume ()
	{
		Paused_ = false;
		ReqTimer_->start (Timeout_);
	}

	void QueueManager::exec ()
	{
		if (Queue_.isEmpty ())
			return;

		if (Paused_)
			return;

		const auto& pair = Queue_.takeFirst ();
		if (pair.second && !*pair.second)
		{
			exec ();
			return;
		}

		pair.first ();
		LastRequest_ = QDateTime::currentDateTime ();

		if (!Queue_.isEmpty ())
			ReqTimer_->start (Timeout_);
	}
}
}
