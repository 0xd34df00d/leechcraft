/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <QMetaObject>
#include <util/sll/raiisignalconnection.h>
#include "../threadsconfig.h"

class QProcess;

namespace LC::Util::detail
{
	struct UTIL_THREADS_API ProcessAwaiter
	{
		QProcess& Process_;

		RaiiSignalConnection FinishedConn_ {};
		RaiiSignalConnection ErrorConn_ {};

		bool await_ready () const noexcept;
		void await_suspend (std::coroutine_handle<> handle) noexcept;
		void await_resume () const noexcept;
	};
}

namespace LC
{
	UTIL_THREADS_API Util::detail::ProcessAwaiter operator co_await (QProcess& process);
}
