/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <exception>
#include <ranges>
#include <type_traits>
#include <QVector>
#include <util/sll/void.h>
#include "../threadsconfig.h"
#include "task.h"

namespace LC::Util
{
	namespace detail
	{
		UTIL_THREADS_API void HandleSubsequentException ();

		inline void SaveFirst (std::exception_ptr& ex)
		{
			if (!ex)
				ex = std::current_exception ();
			else
				HandleSubsequentException ();
		}

		template<
				typename T,
				template<typename> typename... Exts,
				bool IsRet = !std::is_void_v<T>,
				typename RetType = std::conditional_t<IsRet, QVector<T>, void>
			>
		Task<RetType, Exts...> JoinAll (QVector<Task<T, Exts...>> tasks, std::exception_ptr ex)
		{
			std::conditional_t<IsRet, QVector<T>, Void> result;
			if constexpr (IsRet)
				result.reserve (tasks.size ());

			for (auto& task : tasks)
				try
				{
					if constexpr (IsRet)
						result << co_await task;
					else
						co_await task;
				}
			catch (...)
			{
				SaveFirst (ex);
			}

			if (ex)
				std::rethrow_exception (ex);

			if constexpr (IsRet)
				co_return result;
		}
	}

	/** @brief Awaits all the `tasks`, collecting their results in order.
	 *
	 * Every task is awaited even if some of them fail. Then the first
	 * exception (in the order of `tasks`) is rethrown, and the other
	 * ones are logged and dropped, except for the ContextDeadException
	 * ones, which are expected to come in bulk.
	 *
	 * @return A task yielding the `QVector` of the results, or a `void` task
	 * for `void`-returning `tasks`.
	 */
	template<typename T, template<typename> typename... Exts>
	auto InParallel (QVector<Task<T, Exts...>> tasks)
	{
		return detail::JoinAll (std::move (tasks), {});
	}

	/** @brief Same as `InParallel(QVector<Task<T, Exts...>>)` for a braced list.
	 */
	template<typename T, template<typename> typename... Exts>
	auto InParallel (std::initializer_list<Task<T, Exts...>> tasks)
	{
		return detail::JoinAll (QVector<Task<T, Exts...>> { tasks }, {});
	}

	/** @brief Starts a task per input and awaits them all.
	 *
	 * The tasks are created in order, and then awaited as by
	 * `InParallel(QVector<Task<T, Exts...>>)`. Should `mkTask` throw, no
	 * further tasks are created, the already created ones are still
	 * awaited, and then the exception is rethrown.
	 */
	template<
			std::ranges::range Inputs,
			typename F,
			typename... MkTaskArgs
		>
	auto InParallel (Inputs inputs, F mkTask, MkTaskArgs&&... mkTaskArgs)
	{
		QVector<decltype (std::invoke (mkTask, std::move (*inputs.begin ()), mkTaskArgs...))> tasks;
		if constexpr (std::ranges::sized_range<Inputs>)
			tasks.reserve (std::ranges::size (inputs));

		std::exception_ptr ex;
		try
		{
			for (auto&& input : inputs)
				tasks << std::invoke (mkTask, std::move (input), mkTaskArgs...);
		}
		catch (...)
		{
			ex = std::current_exception ();
		}
		return detail::JoinAll (std::move (tasks), ex);
	}

	/** @brief Awaits the `tasks` of different result types.
	 *
	 * @return A task yielding the std::tuple of the results.
	 */
	template<typename... Ts, template<typename> typename... Exts>
	Task<std::tuple<Ts...>, Exts...> InParallel (Task<Ts, Exts...>... tasks)
	{
		// TODO C++26 template for to account for `tasks` exceptions here
		co_return std::tuple<Ts...> { co_await tasks... };
	}

	/** @brief Starts `count` tasks via `taskFactory` and awaits them all.
	 *
	 * The tasks are created and awaited as by `InParallel()`, including
	 * the handling of a throwing factory.
	 */
	auto NCopies (qsizetype count, auto taskFactory)
	{
		return InParallel (QVector<Void> { count, Void {} }, [&] (Void) { return taskFactory (); });
	}
}
