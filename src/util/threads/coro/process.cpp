/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "process.h"
#include <QProcess>
#include <QtDebug>
#include <util/sll/visitor.h>

namespace LC::Util
{
	QDebug operator<< (QDebug dbg, const ProcessFailedToStart& failed)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "ProcessFailedToStart { " << failed.Error_ << " }";
		return dbg;
	}

	QDebug operator<< (QDebug dbg, const ProcessCrashed& crashed)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "ProcessCrashed { code: " << crashed.Code_ << " }";
		return dbg;
	}

	QDebug operator<< (QDebug dbg, const ProcessExited& exited)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "ProcessExited { code: " << exited.Code_ << " }";
		return dbg;
	}

	QDebug operator<< (QDebug dbg, const ProcessOutcome& outcome)
	{
		Visit (outcome, [&dbg] (const auto& alternative) { dbg << alternative; });
		return dbg;
	}
}

namespace LC::Util::detail
{
	bool ProcessAwaiter::await_ready () const noexcept
	{
		return Process_.state () == QProcess::NotRunning;
	}

	void ProcessAwaiter::await_suspend (std::coroutine_handle<> handle)
	{
		const auto resumeLater = [this, handle]
		{
			if (!await_ready ())
				return;

			QObject::disconnect (std::move (FinishedConn_).Release ());
			QObject::disconnect (std::move (ErrorConn_).Release ());
			QMetaObject::invokeMethod (&CoroResumeGuard_, handle, Qt::QueuedConnection);
		};
		FinishedConn_ = QObject::connect (&Process_, &QProcess::finished, resumeLater);
		ErrorConn_ = QObject::connect (&Process_, &QProcess::errorOccurred, resumeLater);
	}

	ProcessOutcome ProcessAwaiter::await_resume () const
	{
		if (Process_.error () == QProcess::FailedToStart)
			return ProcessFailedToStart { Process_.errorString () };

		if (Process_.exitStatus () == QProcess::CrashExit)
			return ProcessCrashed { Process_.exitCode () };

		return ProcessExited { Process_.exitCode () };
	}
}

namespace LC
{
	UTIL_THREADS_API Util::detail::ProcessAwaiter operator co_await (QProcess& reply)
	{
		return { reply };
	}
}
