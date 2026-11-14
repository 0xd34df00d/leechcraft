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
#include <QObject>
#include <util/sll/void.h>
#include "channel.h"

namespace LC::Util
{
	/** @brief A mutex for coroutines, held across suspension points.
	 *
	 * Awaiting `Lock()` yields a guard as soon as no other coroutine
	 * holds the mutex, and the mutex stays locked until that guard dies.
	 * This serializes the operations that would otherwise interleave at
	 * their `co_await`s:
	 * ```cpp
	 * ContextTask<void> Widget::save ()
	 * {
	 * 	co_await AddContext { CoroContext_ };
	 * 	const auto guard = co_await Operations_.Lock ();
	 * 	const auto& text = co_await Editor_->GetContents ();
	 * 	DraftId_ = Storage_->Save (DraftId_, text);
	 * }
	 * ```
	 *
	 * The contract:
	 * - *Not recursive.*
	 *   A coroutine holding the mutex shall neither lock it again nor
	 *   await a coroutine that does: it would wait forever.
	 * - *No reentrancy.*
	 *   A waiting coroutine never runs inside the destructor of a guard:
	 *   it is resumed through the event queue of its thread.
	 * - *Cancellation.*
	 *   A coroutine cancelled while it waits never gets the mutex, and a
	 *   guard destroyed by the cancellation of its coroutine unlocks.
	 * - *No order.*
	 *   Which of the waiting coroutines gets the mutex next is not
	 *   specified.
	 * - *Lifetime.*
	 *   The mutex shall outlive its guards and the coroutines waiting
	 *   for it.
	 */
	class CoroMutex
	{
		Channel<Void> Token_;
	public:
		class [[nodiscard]] Guard
		{
			friend class CoroMutex;

			CoroMutex& Mutex_;

			explicit Guard (CoroMutex& mutex)
			: Mutex_ { mutex }
			{
			}
		public:
			Guard (const Guard&) = delete;
			Guard (Guard&&) = delete;
			Guard& operator= (const Guard&) = delete;
			Guard& operator= (Guard&&) = delete;

			~Guard ()
			{
				Mutex_.Token_.Send (Void {});
			}
		};
	private:
		struct Awaiter
		{
			CoroMutex& Mutex_;
			decltype (std::declval<Channel<Void>&> ().Receive ()) Receiver_;

			bool await_ready ()
			{
				return Receiver_.await_ready ();
			}

			bool await_suspend (std::coroutine_handle<> handle)
			{
				return Receiver_.await_suspend (handle);
			}

			Guard await_resume ()
			{
				Receiver_.await_resume ();
				return Guard { Mutex_ };
			}
		};
	public:
		CoroMutex ()
		{
			Token_.Send (Void {});
		}

		/** @brief Whether the mutex is held by a coroutine or is on its way to a waiting one.
		 */
		bool IsLocked () const
		{
			return Token_.IsEmpty ();
		}

		/** @brief Returns the awaitable yielding the `Guard` once the mutex is locked for the awaiting coroutine.
		 */
		[[nodiscard]] Awaiter Lock ()
		{
			return { *this, Token_.Receive () };
		}
	};
}
