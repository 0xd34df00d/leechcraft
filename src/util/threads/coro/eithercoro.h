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
#include "either.h"

namespace LC::Util::detail
{
	struct EitherLeanCoroAbort {};
}

template<typename L, typename R, typename... Args>
struct std::coroutine_traits<LC::Util::Either<L, R>, Args...>
{
	using EitherType = LC::Util::Either<L, R>;

	struct State
	{
		std::optional<EitherType> Ret_ {};
		std::exception_ptr Exc_ {};
	};

	struct promise_type;

	struct Wrapper
	{
		State State_;
		promise_type& Promise_;

		explicit Wrapper (promise_type& promise)
		: Promise_ { promise }
		{
			promise.Wrapper_ = this;
		}

		Wrapper (Wrapper&& other) noexcept
		: State_ { std::move (other.State_) }
		, Promise_ { other.Promise_ }
		{
			Promise_.Wrapper_ = this;
		}

		Wrapper (const Wrapper&) = delete;
		Wrapper& operator= (const Wrapper&) = delete;
		Wrapper& operator= (Wrapper&&) = delete;

		explicit (false) operator EitherType ()
		{
			if (State_.Exc_)
			{
				try
				{
					std::rethrow_exception (State_.Exc_);
				}
				catch (const LC::Util::detail::EitherLeanCoroAbort&) {}
			}
			return std::move (*State_.Ret_);
		}
	};

	struct promise_type
	{
		constexpr std::suspend_never initial_suspend () const noexcept { return {}; }
		constexpr std::suspend_never final_suspend () const noexcept { return {}; }

		constexpr static bool IsVoid = false;

		Wrapper *Wrapper_ = nullptr;

		template<typename U = R>
		void return_value (U&& val)
		{
			Wrapper_->State_.Ret_.emplace (std::forward<U> (val));
		}

		void return_value (LC::Util::Left<L>&& val)
		{
			Wrapper_->State_.Ret_.emplace (std::move (val));
		}

		void unhandled_exception ()
		{
			Wrapper_->State_.Exc_ = std::current_exception ();
		}

		Wrapper get_return_object ()
		{
			return Wrapper { *this };
		}

		template<typename T>
		auto await_transform (T&& either) const
		{
			return SimpleAwaiter<T> { std::forward<T> (either) };
		}

		template<typename T>
		struct SimpleAwaiter
		{
			T Either_;

			constexpr static auto IsOwning = !std::is_lvalue_reference_v<T>;
			using R_t = std::decay_t<T>::R_t;

			bool await_ready () const noexcept
			{
				return Either_.IsRight ();
			}

			[[noreturn]]
			void await_suspend (std::coroutine_handle<promise_type> handle)
			{
				handle.promise ().Wrapper_->State_.Ret_.emplace (std::forward_like<T> (Either_.GetLeft ()));
				throw LC::Util::detail::EitherLeanCoroAbort {};
			}

			std::conditional_t<IsOwning, R_t, const R_t&> await_resume ()
			{
				return std::forward_like<T> (Either_.GetRight ());
			}
		};
	};
};
