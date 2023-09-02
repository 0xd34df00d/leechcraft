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

	namespace detail
	{
		template<typename F, typename Input>
		struct ParallelTraits
		{
			using TaskType_t = std::invoke_result_t<F, Input>;

			using OrigResultType_t = typename TaskType_t::ResultType_t;
			using ResultType_t = TaskType_t::template ReplaceResult_t<QVector<OrigResultType_t>>;
		};
	}

	template<typename Input, typename F>
	auto InParallel (QVector<Input>&& inputs, F&& mkTask) -> detail::ParallelTraits<F, Input>::ResultType_t
	{
		QVector<typename detail::ParallelTraits<F, Input>::OrigResultType_t> result;
		for (auto&& input : inputs)
			result << co_await mkTask (std::move (input));
		co_return result;
	}

	template<typename Input, typename F>
	auto InParallelSemigroup (QVector<Input>&& inputs, F&& mkTask) -> detail::ParallelTraits<F, Input>::TaskType_t
	{
		typename detail::ParallelTraits<F, Input>::OrigResultType_t result;
		for (auto&& input : inputs)
			result += co_await mkTask (std::move (input));
		co_return result;
	}

	template<typename... Ts, template<typename> typename... Exts>
	Task<std::tuple<Ts...>, Exts...> InParallel (Task<Ts, Exts...>... tasks)
	{
		co_return std::tuple<Ts...> { co_await tasks... };
	}
}
