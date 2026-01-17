/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <util/sll/either.h>

template<typename L, typename R, typename... Args>
struct std::coroutine_traits<LC::Util::Either<L, R>, Args...>
{
	using EitherType = LC::Util::Either<L, R>;

	struct State
	{
		std::optional<EitherType> Ret_ {};
		std::exception_ptr Exc_ {};
		std::coroutine_handle<> Handle_;
	};

	struct Wrapper
	{
		State& State_;
		bool Die_ = false;

		~Wrapper ()
		{
			if (Die_)
				State_.Handle_.destroy ();
		}

		explicit (false) operator EitherType ()
		{
			Die_ = true;
			if (State_.Exc_)
				try
				{
					std::rethrow_exception (State_.Exc_);
				}
				catch (const LC::Util::detail::EitherFailureAbort&)
				{
					// coro terminated early on a `Left` value
					// so ignore it and return
				}
			return State_.Ret_.value ();
		}
	};

	struct promise_type
	{
		constexpr std::suspend_never initial_suspend () const noexcept { return {}; }
		constexpr std::suspend_always final_suspend () const noexcept { return {}; }

		constexpr static bool IsVoid = false;

		State State_ { .Handle_ = std::coroutine_handle<promise_type>::from_promise (*this) };

		template<typename U = R>
		void return_value (U&& val)
		{
			State_.Ret_.emplace (std::forward<U> (val));
		}

		void return_value (LC::Util::Left<L>&& val)
		{
			State_.Ret_.emplace (std::move (val));
		}

		void unhandled_exception ()
		{
			State_.Exc_ = std::current_exception ();
		}

		Wrapper get_return_object ()
		{
			return { State_ };
		}
	};
};
