/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <mutex>
#include "task.h"
#include "context.h"

namespace LC::Util
{
	template<typename>
	struct ThreadSafetyExtension
	{
		static constexpr bool IsLockingHandler = true;
		static constexpr bool IsThreadSafe = true;

		mutable std::mutex Mutex_;

		void lock () const
		{
			Mutex_.lock ();
		}

		void unlock () const
		{
			Mutex_.unlock ();
		}
	};

	template<typename T = void>
	using ThreadSafeTask = Task<T, ThreadSafetyExtension>;

	template<typename T = void>
	using ThreadSafeContextTask = Task<T, ThreadSafetyExtension, ContextExtension>;

	template<typename T>
	concept IsThreadSafe = requires { T::IsThreadSafe; };
}
