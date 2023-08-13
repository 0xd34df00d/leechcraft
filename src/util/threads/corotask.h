/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <utility>

namespace LC::Util
{
	namespace detail
	{
		template<typename R>
		struct PromiseRet
		{
			constexpr static bool IsVoid = false;

			std::optional<R> Ret_;

			template<typename U>
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

		template<typename Promise>
		struct TaskAwaiter
		{
			using Handle_t = std::coroutine_handle<Promise>;
			Handle_t Handle_;

			bool await_ready () const noexcept
			{
				const auto& promise = Handle_.promise ();
				if constexpr (Promise::IsVoid)
					return promise.Done_;
				else
					return static_cast<bool> (promise.Ret_);
			}

			void await_suspend (std::coroutine_handle<> handle)
			{
				Handle_.promise ().WaitingHandles_.push_back (handle);
			}

			auto await_resume () const noexcept
			{
				if constexpr (!Promise::IsVoid)
					return *Handle_.promise ().Ret_;
			}
		};
	}

	template<typename R>
	class Task
	{
	public:
		struct promise_type;
	private:
		using Handle_t = std::coroutine_handle<promise_type>;
		Handle_t Handle_;
	public:
		struct promise_type : detail::PromiseRet<R>
		{
			QVector<std::coroutine_handle<>> WaitingHandles_;

			Task get_return_object ()
			{
				return Task { Handle_t::from_promise (*this) };
			}

			std::suspend_never initial_suspend () const noexcept { return {}; }
			std::suspend_always final_suspend () const noexcept { return {}; }
			void unhandled_exception () {}
		};

		explicit Task (const std::coroutine_handle<promise_type>& handle)
		: Handle_ { handle }
		{
		}

		Task (const Task&) = delete;
		Task& operator= (const Task&) = delete;

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
}

