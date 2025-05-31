/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/either.h>
#include "task.h"

namespace LC::Util
{
	struct IgnoreLeft {};

	namespace detail
	{
		template<typename Promise>
		[[noreturn]]
		void TerminateLeftyCoroutine (std::coroutine_handle<Promise> handle, const auto& error)
		{
			auto& promise = handle.promise ();
			if constexpr (Promise::IsVoid)
				promise.return_void ();
			else
				promise.return_value (Left { error });

			throw EitherFailureAbort {};
		}

		template<typename L, typename R, typename ErrorHandler>
		struct EitherAwaiter
		{
			Either<L, R> Either_;
			ErrorHandler Handler_;

			using HandlerReturn_t = decltype (Handler_ (Either_.GetLeft ()));

			bool await_ready () const noexcept
			{
				return Either_.IsRight ();
			}

			void await_suspend (auto handle)
			{
				if constexpr (std::is_same_v<void, HandlerReturn_t>)
				{
					Handler_ (Either_.GetLeft ());
					TerminateLeftyCoroutine (handle, Either_.GetLeft ());
				}
				else if constexpr (std::is_same_v<IgnoreLeft, HandlerReturn_t>)
				{
					static_assert (std::is_default_constructible_v<R>);
					Handler_ (Either_.GetLeft ());
					Either_ = R {};
				}
				else
					TerminateLeftyCoroutine (handle, Handler_ (Either_.GetLeft ()));
			}

			R await_resume () const noexcept
			{
				return Either_.GetRight ();
			}
		};
	}

	template<typename L, typename R, typename F>
		requires std::invocable<F, const L&>
	detail::EitherAwaiter<L, R, F> WithHandler (const Either<L, R>& either, F&& errorHandler)
	{
		return { either, std::forward<F> (errorHandler) };
	}
}

namespace LC
{
	template<typename L, typename R>
	Util::detail::EitherAwaiter<L, R, std::identity> operator co_await (const Util::Either<L, R>& either)
	{
		return { either, {} };
	}
}
