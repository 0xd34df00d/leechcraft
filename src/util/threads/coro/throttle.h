/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <chrono>
#include <QElapsedTimer>
#include <QTimer>
#include <QVector>
#include "../threadsconfig.h"

namespace LC::Util
{
	class UTIL_THREADS_API Throttle
	{
		QTimer Timer_;
		QElapsedTimer LastInvocation_;
		std::chrono::milliseconds Interval_;

		QVector<std::coroutine_handle<>> Queue_;
	public:
		explicit Throttle (std::chrono::milliseconds);

		std::chrono::milliseconds GetInterval () const;

		bool await_ready ();
		void await_suspend (std::coroutine_handle<>);
		void await_resume () const;
	};

	using Throttle_ptr = std::shared_ptr<Throttle>;
}
