/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVector>
#include "task.h"

namespace LC::Util
{
	template<typename T, template<typename> typename... Exts>
	Task<QVector<T>, Exts...> InParallel (QVector<Task<T, Exts...>> tasks)
	{
		QVector<T> result;
		for (auto& task : tasks)
			result << co_await task;
		co_return result;
	}

	template<typename... Ts, template<typename> typename... Exts>
	Task<std::tuple<Ts...>, Exts...> InParallel (Task<Ts, Exts...>... tasks)
	{
		co_return std::tuple<Ts...> { co_await tasks... };
	}
}
