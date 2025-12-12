/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>
#include <type_traits>
#include <QEventLoop>
#include "task.h"

namespace LC::Util
{
	template<typename T, template<typename> typename... Extensions>
	T GetTaskResult (Task<T, Extensions...> task)
	{
		constexpr bool isVoid = std::is_same_v<T, void>;
		std::conditional_t<isVoid, void*, std::unique_ptr<T>> result;

		std::exception_ptr exception;

		QEventLoop loop;
		std::atomic_bool done = false;
		[] (auto task, auto& result, auto& exception, auto& done, auto& loop) -> Task<void>
		{
			try
			{
				if constexpr (isVoid)
					co_await task;
				else
					result = std::make_unique<T> (co_await task);
			}
			catch (...)
			{
				exception = std::current_exception ();
			}
			done = true;
			loop.quit ();
		} (task, result, exception, done, loop);
		if (!done)
			loop.exec ();

		if (exception)
			std::rethrow_exception (exception);

		if constexpr (!isVoid)
			return *result;
	}
}
