/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "workerthreadbase.h"
#include <util/sll/slotclosure.h>

namespace LC
{
namespace Util
{
	void WorkerThreadBase::SetPaused (bool paused)
	{
		if (paused == IsPaused_)
			return;

		IsPaused_ = paused;
		if (!paused)
			emit rotateFuncs ();
	}

	size_t WorkerThreadBase::GetQueueSize ()
	{
		QMutexLocker locker { &FunctionsMutex_ };
		return Functions_.size ();
	}

	void WorkerThreadBase::run ()
	{
		SlotClosure<NoDeletePolicy> rotator
		{
			[this] { RotateFuncs (); },
			this,
			SIGNAL (rotateFuncs ()),
			nullptr
		};

		Initialize ();

		RotateFuncs ();

		QThread::run ();

		Cleanup ();
	}

	void WorkerThreadBase::RotateFuncs ()
	{
		if (IsPaused_)
			return;

		decltype (Functions_) funcs;

		{
			QMutexLocker locker { &FunctionsMutex_ };

			using std::swap;
			swap (funcs, Functions_);
		}

		for (const auto& func : funcs)
			func ();
	}
}
}
