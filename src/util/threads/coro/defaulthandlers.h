/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>
#include <coroutine>
#include <stdexcept>

namespace LC::Util::detail
{
	template<typename E>
	concept IsAwaiterHandler = E::IsAwaiterHandler;

	template<typename...>
	struct DefaultAwaiterHandler
	{
		std::atomic<std::coroutine_handle<>> Continuation_ {};

		struct DoubleAwaitError : std::logic_error
		{
			DoubleAwaitError ()
			: std::logic_error { "double await" }
			{
			}
		};

		void AddAwaiter (std::coroutine_handle<> handle)
		{
			if (this->Continuation_.exchange (handle))
				throw DoubleAwaitError {};
		}

		void RemoveAwaiter (std::coroutine_handle<>) noexcept
		{
			this->Continuation_.exchange ({});
		}

		auto GetAwaiters (this auto&& self) noexcept
		{
			return self.Continuation_.exchange ({});
		}
	};

	template<typename... Extensions>
		requires (IsAwaiterHandler<Extensions> || ...)
	struct DefaultAwaiterHandler<Extensions...> {};


	template<typename E>
	concept IsLockingHandler = E::IsLockingHandler;

	template<typename...>
	struct DefaultLockingHandler
	{
		static void lock () {}
		static void unlock () {}
	};

	template<typename... Extensions>
		requires (IsLockingHandler<Extensions> || ...)
	struct DefaultLockingHandler<Extensions...> {};


	template<typename E>
	concept IsResumeValueHandler = E::IsResumeValueHandler;

	template<typename...>
	struct DefaultResumeValueHandler
	{
		template<typename R>
		static R&& ResumeValue (R& ret) noexcept
		{
			return std::move (ret);
		}
	};

	template<typename... Extensions>
		requires (IsResumeValueHandler<Extensions> || ...)
	struct DefaultResumeValueHandler<Extensions...> {};
}
