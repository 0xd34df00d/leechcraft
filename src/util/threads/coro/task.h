/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <exception>
#include <utility>
#include <QVector>
#include "finalsuspender.h"
#include "taskfwd.h"

namespace LC::Util
{
	namespace detail
	{
		template<typename R>
		struct PromiseRet
		{
			using ReturnType_t = R;

			constexpr static bool IsVoid = false;

			std::optional<R> Ret_;

			template<typename U = R>
			void return_value (U&& val)
			{
				Ret_.emplace (std::forward<U> (val));
			}
		};

		template<>
		struct PromiseRet<void>
		{
			constexpr static bool IsVoid = true;

			bool Done_ = false;

			void return_void () noexcept
			{
				Done_ = true;
			}
		};

		struct EitherFailureAbort final : std::exception {};

		template<typename Promise>
		struct TaskAwaiter
		{
			using Handle_t = std::coroutine_handle<Promise>;
			Handle_t Handle_;

			bool await_ready () const noexcept
			{
				const auto& promise = Handle_.promise ();
				if (promise.Exception_)
					return true;

				if constexpr (Promise::IsVoid)
					return promise.Done_;
				else
					return static_cast<bool> (promise.Ret_);
			}

			void await_suspend (std::coroutine_handle<> handle)
			{
				Handle_.promise ().WaitingHandles_.push_back (handle);
			}

			auto await_resume () const
			{
				const auto& promise = Handle_.promise ();
				if (promise.Exception_)
					try
					{
						std::rethrow_exception (promise.Exception_);
					}
					catch (const EitherFailureAbort&)
					{
					}

				if constexpr (!Promise::IsVoid)
					return *promise.Ret_;
			}
		};
	}

	template<typename R, template<typename> typename... Extensions>
	class Task
	{
	public:
		struct promise_type;
	private:
		using Handle_t = std::coroutine_handle<promise_type>;
		Handle_t Handle_;
	public:
		struct promise_type : detail::PromiseRet<R>
							, Extensions<promise_type>...
		{
			size_t Refs_ = 1; // TODO make thread-safe
			QVector<std::coroutine_handle<>> WaitingHandles_ {};
			std::exception_ptr Exception_ {};

			auto GetAddress () { return Handle_t::from_promise (*this).address (); }

			Task get_return_object ()
			{
				return Task { Handle_t::from_promise (*this) };
			}

			std::suspend_never initial_suspend () const noexcept { return {}; }

			auto final_suspend () noexcept
			{
				([this]
				{
					using Base = Extensions<promise_type>;
					if constexpr (requires (Base t) { t.FinalSuspend (); })
						Base::FinalSuspend ();
				} (), ...);
				return detail::FinalSuspender<promise_type> { *this };
			}

			void unhandled_exception ()
			{
				Exception_ = std::current_exception ();
			}

			void IncRef ()
			{
				++Refs_;
			}

			void DecRef ()
			{
				if (!--Refs_)
					Handle_t::from_promise (*this).destroy ();
			}
		};

		using ResultType_t = R;

		template<typename RR>
		using ReplaceResult_t = Task<RR, Extensions...>;

		explicit Task (const std::coroutine_handle<promise_type>& handle)
		: Handle_ { handle }
		{
			if (handle)
				handle.promise ().IncRef ();
		}

		~Task () noexcept
		{
			if (Handle_)
				Handle_.promise ().DecRef ();
		}

		Task (const Task& other)
		: Handle_ { other.Handle_ }
		{
			if (Handle_)
				Handle_.promise ().IncRef ();
		}

		Task& operator= (const Task& other)
		{
			Task task { other };
			*this = std::move (task);
			return *this;
		}

		Task (Task&& other) noexcept
		{
			std::swap (Handle_, other.Handle_);
		}

		Task& operator= (Task&& other) noexcept
		{
			std::swap (Handle_, other.Handle_);
			return *this;
		}

		auto operator co_await () const noexcept
		{
			return detail::TaskAwaiter<promise_type> { Handle_ };
		}
	};

	namespace detail
	{
		template<typename R, template<typename> typename... Extensions>
		struct GetPromise
		{
			using Promise = typename Task<R, Extensions...>::promise_type;
			Promise *Promise_ = nullptr;

			bool await_ready () const noexcept { return false; }

			bool await_suspend (std::coroutine_handle<Promise> handle) const noexcept
			{
				Promise_ = &handle.promise ();
				return false;
			}

			decltype (auto) await_resume () const noexcept
			{
				return *Promise_;
			}
		};
	}
}
