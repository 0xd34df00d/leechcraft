/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <variant>
#include <QObject>
#include <QString>
#include <util/sll/raiisignalconnection.h>
#include "../threadsconfig.h"

class QDebug;
class QProcess;

namespace LC::Util
{
	struct ProcessFailedToStart
	{
		QString Error_;

		auto operator<=> (const ProcessFailedToStart&) const = default;
	};

	struct ProcessCrashed
	{
		int Code_;

		auto operator<=> (const ProcessCrashed&) const = default;
	};

	struct ProcessExited
	{
		int Code_;

		auto operator<=> (const ProcessExited&) const = default;
	};

	using ProcessOutcome = std::variant<ProcessFailedToStart, ProcessCrashed, ProcessExited>;

	UTIL_THREADS_API QDebug operator<< (QDebug, const ProcessFailedToStart&);
	UTIL_THREADS_API QDebug operator<< (QDebug, const ProcessCrashed&);
	UTIL_THREADS_API QDebug operator<< (QDebug, const ProcessExited&);
	UTIL_THREADS_API QDebug operator<< (QDebug, const ProcessOutcome&);

	namespace detail
	{
		struct UTIL_THREADS_API ProcessAwaiter
		{
			QProcess& Process_;

			QObject CoroResumeGuard_ {};

			RaiiSignalConnection FinishedConn_ {};
			RaiiSignalConnection ErrorConn_ {};

			bool await_ready () const noexcept;
			void await_suspend (std::coroutine_handle<> handle);
			ProcessOutcome await_resume () const;
		};
	}
}

namespace LC
{
	UTIL_THREADS_API Util::detail::ProcessAwaiter operator co_await (QProcess& process);
}
