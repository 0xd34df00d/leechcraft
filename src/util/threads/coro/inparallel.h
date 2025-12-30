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
	template<
			typename T,
			template<typename> typename... Exts,
			template<typename...> typename Cont = std::initializer_list
		>
	Task<QVector<T>, Exts...> InParallel (Cont<Task<T, Exts...>> tasks)
	{
		QVector<T> result;
		for (auto& task : tasks)
			result << co_await task;
		co_return result;
	}

	template<
			template<typename> typename... Exts,
			template<typename...> typename Cont = std::initializer_list
		>
	Task<void, Exts...> InParallel (Cont<Task<void, Exts...>> tasks)
	{
		for (auto& task : tasks)
			co_await task;
	}

	template<
			typename Inputs,
			typename F,
			typename... MkTaskArgs,
			typename Task = std::invoke_result_t<F&,
					std::add_rvalue_reference_t<typename std::decay_t<Inputs>::value_type>,
					MkTaskArgs...
				>,
			bool IsVoid = std::is_same_v<typename Task::ResultType_t, void>
		>
	auto InParallel (Inputs inputs, F mkTask, MkTaskArgs... mkTaskArgs) ->
			std::conditional_t<
				IsVoid,
				typename Task::template ReplaceResult_t<void>,
				typename Task::template ApplyResult_t<QVector>
			>
	{
		QVector<Task> tasks;
		for (auto&& input : inputs)
			tasks << mkTask (std::move (input), mkTaskArgs...);

		if constexpr (IsVoid)
			for (const auto& task : tasks)
				co_await task;
		else
		{
			QVector<typename Task::ResultType_t> result;
			for (const auto& task : tasks)
				result << co_await task;
			co_return result;
		}
	}

	template<typename... Ts, template<typename> typename... Exts>
	Task<std::tuple<Ts...>, Exts...> InParallel (Task<Ts, Exts...>... tasks)
	{
		co_return std::tuple<Ts...> { co_await tasks... };
	}

	auto NCopies (size_t count, auto taskFactory, std::function<void ()> finalizer = {})
			-> decltype (taskFactory ())::template ApplyResult_t<QVector>
		requires (!std::is_same_v<void, typename decltype (taskFactory ())::ResultType_t>)
	{
		using Task_t = decltype (taskFactory ());

		QVector<Task_t> tasks;
		std::generate_n (std::back_inserter (tasks), count, taskFactory);

		QVector<typename Task_t::ResultType_t> results;
		for (auto& task : tasks)
			results << co_await task;
		if (finalizer)
			finalizer ();
		co_return results;
	}

	auto NCopies (size_t count, auto taskFactory, std::function<void ()> finalizer = {})
			-> decltype (taskFactory ())::template ReplaceResult_t<void>
		requires (std::is_same_v<void, typename decltype (taskFactory ())::ResultType_t>)
	{
		using Task_t = decltype (taskFactory ());

		QVector<Task_t> tasks;
		std::generate_n (std::back_inserter (tasks), count, taskFactory);
		for (auto& task : tasks)
			co_await task;

		if (finalizer)
			finalizer ();
	}
}
