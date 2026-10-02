/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fetchqueue.h"
#include <QTimer>
#include <QtDebug>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	FetchQueue::FetchQueue (std::function<void (const QString&, bool)> func,
			int timeout, int perShot, QObject *parent)
	: QObject (parent)
	, FetchTimer_ (new QTimer (this))
	, FetchFunction_ (func)
	, PerShot_ (perShot)
	{
		FetchTimer_->setInterval (timeout);
		connect (FetchTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleFetch ()));
	}

	void FetchQueue::Schedule (const QString& string, FetchQueue::Priority prio, bool report)
	{
		if (report)
			Reports_ << string;

		if (Queue_.contains (string))
			return;

		switch (prio)
		{
		case PHigh:
			Queue_.prepend (string);
			break;
		case PLow:
			Queue_.append (string);
			break;
		}

		if (!FetchTimer_->isActive ())
		{
			QTimer::singleShot (100,
					this,
					SLOT (handleFetch ()));
			FetchTimer_->start ();
		}
	}

	void FetchQueue::Clear ()
	{
		Queue_.clear ();
		Reports_.clear ();

		if (FetchTimer_->isActive ())
			FetchTimer_->stop ();
	}

	void FetchQueue::handleFetch ()
	{
		int num = std::min (PerShot_, Queue_.size ());
		while (num--)
		{
			const auto& str = Queue_.takeFirst ();
			FetchFunction_ (str, Reports_.remove (str));
		}

		if (Queue_.isEmpty () &&
				FetchTimer_->isActive ())
			FetchTimer_->stop ();
	}
}
}
}
