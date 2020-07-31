/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progresslistener.h"

namespace LC
{
namespace Snails
{
	ProgressListener::ProgressListener (QObject *parent)
	: QObject { parent }
	{
	}

	void ProgressListener::Increment ()
	{
		progress (LastProgress_ + 1, LastTotal_);
	}

	bool ProgressListener::cancel () const
	{
		return false;
	}

	void ProgressListener::start (const size_t total)
	{
		emit started (total);
		progress (0, total);
	}

	void ProgressListener::progress (const size_t done, const size_t total)
	{
		LastProgress_ = done;
		LastTotal_ = total;

		emit gotProgress (done, total);
	}

	void ProgressListener::stop (const size_t total)
	{
		progress (total, total);
	}

	bool operator< (const ProgressListener_wptr& w1, const ProgressListener_wptr& w2)
	{
		return w1.owner_before (w2);
	}
}
}
