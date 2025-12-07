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
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

namespace LC::Util
{
	template<typename T>
	class Channel
	{
		struct PopAwaiter
		{
			Channel& Ch_;
			std::optional<T> Slot_;
			std::coroutine_handle<> Handle_;
			bool Registered_ = false;

			explicit PopAwaiter (Channel& ch)
			: Ch_ { ch }
			{
			}

			~PopAwaiter ()
			{
				if (Registered_)
				{
					std::lock_guard guard { Ch_.Lock_ };
					std::erase (Ch_.Awaiters_, this);
				}
			}

			bool await_ready () const noexcept
			{
				return false;
			}

			bool await_suspend (std::coroutine_handle<> handle)
			{
				std::lock_guard guard { Ch_.Lock_ };
				if (!Ch_.Elems_.empty ())
				{
					Slot_ = std::move (Ch_.Elems_.front ());
					Ch_.Elems_.pop_front ();
					return false;
				}

				if (Ch_.Closed_)
					return false;

				Ch_.Awaiters_.push_back (this);
				Handle_ = handle;
				Registered_ = true;
				return true;
			}

			std::optional<T> await_resume () noexcept
			{
				return std::move (Slot_);
			}
		};

		mutable std::mutex Lock_;
		std::deque<T> Elems_;
		std::deque<PopAwaiter*> Awaiters_;

		bool Closed_ = false;
	public:
		Channel () = default;

		void Close ()
		{
			std::deque<PopAwaiter*> awaiters;

			{
				std::lock_guard guard { Lock_ };
				Closed_ = true;
				awaiters = std::exchange (Awaiters_, {});
				for (auto awaiter : awaiters)
					awaiter->Registered_ = false;
			}

			for (auto awaiter : awaiters)
				awaiter->Handle_ ();
		}

		template<typename U = T>
		void Push (U&& value)
		{
			PopAwaiter *next = nullptr;
			{
				std::lock_guard guard { Lock_ };
				if (Closed_)
					throw std::runtime_error { "pushing into a closed channel" };

				if (!Awaiters_.empty ())
				{
					next = Awaiters_.front ();
					Awaiters_.pop_front ();
					next->Registered_ = false;
				}
				else
					Elems_.emplace_back (std::forward<U> (value));
			}

			if (next)
			{
				next->Slot_.emplace (std::forward<U> (value));
				next->Handle_ ();
			}
		}

		PopAwaiter Pop ()
		{
			return PopAwaiter { *this };
		}
	};
}
