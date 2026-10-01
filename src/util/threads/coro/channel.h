/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>
#include "channelfwd.h"

namespace LC::Util
{
	template<typename T>
	class Channel
	{
		struct ReceiveAwaiter
		{
			QObject Ctx_;

			Channel& Ch_;
			std::optional<T> Slot_;
			std::coroutine_handle<> Handle_;
			bool Registered_ = false;

			explicit ReceiveAwaiter (Channel& ch)
			: Ch_ { ch }
			{
			}

			~ReceiveAwaiter ()
			{
				if (Registered_)
				{
					std::lock_guard guard { Ch_.Lock_ };
					std::erase (Ch_.Awaiters_, this);
				}

				// channel consumer died while scheduled but without consuming the slot
				if (Slot_)
					Ch_.SendImpl (std::move (*Slot_), SendClosedBehavior::Ignore);
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
				return std::exchange (Slot_, std::nullopt);
			}
		};

		mutable std::mutex Lock_;
		std::deque<T> Elems_;
		std::deque<ReceiveAwaiter*> Awaiters_;

		bool Closed_ = false;
	public:
		using ItemType_t = T;

		Channel () = default;

		Channel (const Channel&) = delete;
		Channel (Channel&&) = delete;
		Channel& operator= (const Channel&) = delete;
		Channel& operator= (Channel&&) = delete;

		void Close ()
		{
			{
				std::lock_guard guard { Lock_ };
				if (std::exchange (Closed_, true))
					return;
			}

			while (const auto next = PopNextAwaiter ())
				Invoke (*next);
		}

		template<typename U = T>
		void Send (U&& value)
		{
			SendImpl (std::forward<U> (value), SendClosedBehavior::Throw);
		}

		bool IsEmpty () const
		{
			std::lock_guard guard { Lock_ };
			return Elems_.empty ();
		}

		ReceiveAwaiter Receive ()
		{
			return ReceiveAwaiter { *this };
		}

		auto operator co_await ()
		{
			return Receive ();
		}
	private:
		enum class SendClosedBehavior : std::uint8_t { Ignore, Throw };

		template<typename U = T>
		void SendImpl (U&& value, SendClosedBehavior closedBehavior)
		{
			const auto next = [&]
			{
				std::lock_guard guard { Lock_ };
				if (Closed_ && closedBehavior == SendClosedBehavior::Throw)
					throw std::runtime_error { "sending into a closed channel" };

				const auto awaiter = PopNextAwaiterUnlocked ();
				if (!awaiter)
					Elems_.emplace_back (std::forward<U> (value));
				return awaiter;
			} ();

			if (next)
			{
				next->Slot_.emplace (std::forward<U> (value));
				Invoke (*next);
			}
		}

		ReceiveAwaiter* PopNextAwaiter ()
		{
			std::lock_guard guard { Lock_ };
			return PopNextAwaiterUnlocked ();
		}

		ReceiveAwaiter* PopNextAwaiterUnlocked ()
		{
			if (Awaiters_.empty ())
				return nullptr;
			auto next = Awaiters_.front ();
			Awaiters_.pop_front ();
			next->Registered_ = false;
			return next;
		}

		void Invoke (ReceiveAwaiter& awaiter)
		{
			QMetaObject::invokeMethod (&awaiter.Ctx_,
					[&awaiter] { awaiter.Handle_ (); },
					Qt::QueuedConnection);
		}
	};
}
