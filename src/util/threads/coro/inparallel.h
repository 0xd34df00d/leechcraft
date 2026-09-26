/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <ranges>
#include <QVector>
#include "task.h"

namespace LC::Util
{
	template<
			typename T,
			template<typename> typename... Exts
		>
	Task<QVector<T>, Exts...> InParallel (QVector<Task<T, Exts...>> tasks)
	{
		QVector<T> result;
		for (auto& task : tasks)
			result << co_await task;
		co_return result;
	}

	template<template<typename> typename... Exts>
	Task<void, Exts...> InParallel (QVector<Task<void, Exts...>> tasks)
	{
		for (auto& task : tasks)
			co_await task;
	}

	template<
			std::ranges::range Inputs,
			typename F,
			typename... MkTaskArgs
		>
	auto InParallel (Inputs inputs, F mkTask, MkTaskArgs&&... mkTaskArgs)
	{
		QVector<decltype (std::invoke (mkTask, std::move (*inputs.begin ()), mkTaskArgs...))> tasks;
		for (auto&& input : inputs)
			tasks << std::invoke (mkTask, std::move (input), mkTaskArgs...);
		return InParallel (std::move (tasks));
	}

	template<typename... Ts, template<typename> typename... Exts>
	Task<std::tuple<Ts...>, Exts...> InParallel (Task<Ts, Exts...>... tasks)
	{
		co_return std::tuple<Ts...> { co_await tasks... };
	}

	auto NCopies (size_t count, auto taskFactory)
	{
		using Task_t = decltype (taskFactory ());

		QVector<Task_t> tasks;
		std::generate_n (std::back_inserter (tasks), count, taskFactory);
		return InParallel (std::move (tasks));
	}

}
