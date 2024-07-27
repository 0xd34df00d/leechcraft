/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "process.h"
#include <QProcess>

namespace LC::Util::detail
{
	bool ProcessAwaiter::await_ready () const noexcept
	{
		return Process_.state () == QProcess::NotRunning;
	}

	void ProcessAwaiter::await_suspend (std::coroutine_handle<> handle) noexcept
	{
		auto finishedConn = QObject::connect (&Process_,
				&QProcess::finished,
				handle);
		QObject::connect (&Process_,
				&QProcess::errorOccurred,
				[=, this]
				{
					if (await_ready ())
					{
						QObject::disconnect (finishedConn);
						handle ();
					}
				});
	}

	void ProcessAwaiter::await_resume () const noexcept
	{
	}
}

namespace LC
{
	UTIL_THREADS_API Util::detail::ProcessAwaiter operator co_await (QProcess& reply)
	{
		return { reply };
	}
}
