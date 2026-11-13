/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <concepts>
#include <coroutine>
#include <optional>
#include <tuple>
#include <util/sll/trackingpointer.h>
#include "taskfwd.h"

namespace LC::Util
{
	template<typename... Args>
	class Callback;

	namespace detail
	{
		template<typename... Args>
		struct CallbackAwaiter
		{
			TrackingPointer<Callback<Args...>> Callback_;

			explicit CallbackAwaiter (Callback<Args...>& cb)
			: Callback_ { &cb }
			{
				if (Callback_->Cont_)
					throw std::runtime_error { "`Callback` is already being awaited on, but it's single-use" };
			}

			CallbackAwaiter (const CallbackAwaiter&) = delete;
			CallbackAwaiter (CallbackAwaiter&&) = delete;
			CallbackAwaiter& operator= (const CallbackAwaiter&) = delete;
			CallbackAwaiter& operator= (CallbackAwaiter&&) = delete;

			~CallbackAwaiter ()
			{
				if (Callback_)
					Callback_->Cont_ = {};
			}

			bool await_ready () const noexcept
			{
				return Callback_->Result_.has_value () || Callback_->Dropped_;
			}

			void await_suspend (std::coroutine_handle<> handle)
			{
				Callback_->Cont_ = handle;
			}

			auto await_resume () noexcept ((std::is_nothrow_move_constructible_v<std::decay_t<Args>> && ...))
			{
				constexpr auto argsCount = sizeof... (Args);
				if constexpr (!argsCount)
					return Callback_->Result_.has_value ();
				else if constexpr (argsCount == 1)
					return std::move (Callback_->Result_).transform ([]<typename T> (T&& tup) { return std::get<0> (std::forward<T> (tup)); });
				else
					return std::move (Callback_->Result_);
			}
		};
	}

	/** @brief Lets a coroutine `co_await` the invocation of a callback.
	 *
	 * Example use:
	 * ```cpp
	 * Callback<QVariant> callback;
	 * page->runJavaScript (script, callback.GetCallback ());
	 * if (const auto result = co_await callback)
	 * 	HandleResult (*result);
	 * else
	 * 	qWarning () << "the callback was destroyed without being invoked";
	 * ```
	 *
	 * `GetCallback()` makes a _functor_ to hand off,
	 * and `co_await`ing the `Callback` yields what the
	 * functor was invoked with (omitting `std::decay_t` on `Args` for
	 * brevity):
	 * - `bool` if there are no `Args`: `true` once it was invoked;
	 * - `std::optional<Args...[0]>` if there is one;
	 * - `std::optional<std::tuple<Args...>>` if there are many.
	 *
	 * A functor whose copies are all gone without an invocation makes that
	 * `false` or `std::nullopt`. The invocation may as well come before the
	 * `co_await`, which then does not suspend.
	 *
	 * The contractual requirements are:
	 * - *Single use.* `GetCallback()` shall be called at most once, and at
	 *   most one coroutine shall await the `Callback` at a time.
	 * - *Threads.* The functor shall be invoked and destroyed on the thread
	 *   the `Callback` lives on, and the `Callback` itself shall be awaited
	 *   on that thread.
	 *
	 * The contractual guarantees are:
	 * - *Threads.* The coroutine resumes on the thread the `Callback` lives
	 *   on, from its event loop.
	 * - *No reentrancy.* The coroutine never runs inside the functor's
	 *   invocation or destruction, so it may freely destroy the callee.
	 * - *Cancellation.* A cancelled coroutine stops awaiting; the result is
	 *   kept for the next `co_await`.
	 * - *Lifetime.* The functor may outlive the `Callback` and does nothing
	 *   then. A coroutine awaiting a `Callback` that gets destroyed is never
	 *   resumed by it.
	 */
	template<typename... Args>
	class Callback : public QObject
	{
		std::optional<std::tuple<std::decay_t<Args>...>> Result_ {};
		std::coroutine_handle<> Cont_ {};
		bool Dropped_ = false;
		bool Used_ = false;

		friend class detail::CallbackAwaiter<Args...>;

		struct Functor
		{
			TrackingPointer<Callback> Callback_;
			std::shared_ptr<void> DroppedMarker_;

			template<std::convertible_to<std::decay_t<Args>>... UArgs>
				requires (sizeof...(Args) == sizeof...(UArgs))
			void operator () (UArgs&&... args)
			{
				if (!Callback_)
					return;

				Callback_->Result_.emplace (std::forward<UArgs> (args)...);
				Callback_->Resume ();
				Callback_.Reset ();
			}
		};
	public:
		Callback () = default;

		Callback (const Callback&) = delete;
		Callback (Callback&&) = delete;
		Callback& operator= (const Callback&) = delete;
		Callback& operator= (Callback&&) = delete;

		auto GetCallback ()
		{
			if (std::exchange (Used_, true))
				throw std::runtime_error { "`Callback` is single-use" };

			struct DroppedMarkingDeleter
			{
				TrackingPointer<Callback> Callback_;

				void operator() (void*) const
				{
					if (Callback_ && !Callback_->Result_)
					{
						Callback_->Dropped_ = true;
						Callback_->Resume ();
					}
				}
			};

			return Functor { this, std::shared_ptr<void> { nullptr, DroppedMarkingDeleter { this } } };
		}
	private:
		void Resume ()
		{
			if (Cont_)
				QMetaObject::invokeMethod (this,
						[this]
						{
							if (const auto cont = std::exchange (Cont_, {}))
								cont ();
						},
						Qt::QueuedConnection);
		}
	};

	template<typename... Args>
	auto GetAwaiter (Callback<Args...>& cb, AwaiterCPO)
	{
		return detail::CallbackAwaiter<Args...> { cb };
	}
}

namespace LC
{
	template<typename... Args>
	auto operator co_await (Util::Callback<Args...>& cb)
	{
		return Util::GetAwaiter (cb, {});
	}
}
