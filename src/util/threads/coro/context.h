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
#include "../threadsconfig.h"
#include "corocontext.h"

namespace LC::Util
{
	namespace detail
	{
		template<typename T>
		concept IsAwaiter = requires (T t)
		{
			t.await_ready ();
			t.await_resume ();
		};

		template<typename T>
		decltype (auto) Awaiter (T&& obj)
		{
			if constexpr (requires { operator co_await (std::forward<T> (obj)); })
				return operator co_await (std::forward<T> (obj));
			else if constexpr (requires { std::forward<T> (obj).operator co_await (); })
				return std::forward<T> (obj).operator co_await ();
			else
			{
				static_assert (IsAwaiter<std::remove_reference_t<T>>, "operand doesn't look like an Awaitable");
				return std::forward<T> (obj);
			}
		}

		UTIL_THREADS_API void CheckDeadContexts (const ContextExtensionBase&);

		template<typename Promise, typename OrigAwaiter>
		struct AwaitableWrapper
		{
			Promise& Promise_;
			OrigAwaiter Orig_;

			bool await_ready ()
			{
				// If the coro is near death due to a context death, awaiting it is futile at best and UB at worst
				// (for instance, if the thing being awaited is guarded by the `CoroContext` who killed the coro).
				return Promise_.HasDeadContexts () || Orig_.await_ready ();
			}

			decltype (auto) await_suspend (auto handle)
			{
				Promise_.Suspended_ = true;
				try
				{
					return Orig_.await_suspend (handle);
				}
				catch (...)
				{
					Promise_.Suspended_ = false;
					throw;
				}
			}

			decltype (auto) await_resume ()
			{
				Promise_.Suspended_ = false;
				CheckDeadContexts (Promise_);
				return Orig_.await_resume ();
			}
		};
	}

	struct [[nodiscard]] AddContext
	{
		CoroContext& Ctx_;

		explicit AddContext (CoroContext& ctx)
		: Ctx_ { ctx }
		{
		}

		bool await_ready () const noexcept
		{
			return false;
		}

		template<typename Promise>
			requires requires { typename Promise::HasContextExtension; }
		bool await_suspend (std::coroutine_handle<Promise> handle)
		{
			// children should die first (as they are registered last)
			Ctx_.Coros_.push_front ({ handle, &handle.promise () });
			const auto it = Ctx_.Coros_.begin ();
			handle.promise ().Contexts_.push_back ({ &Ctx_, it });
			return false;
		}

		void await_resume () const noexcept
		{
		}
	};

	struct UTIL_THREADS_API ContextExtensionBase
	{
		CoroContext::Registrations Contexts_;
		QStringList DeadContexts_;
		bool Suspended_ = false;

		~ContextExtensionBase ()
		{
			for (auto [ctx, thisPos] : Contexts_)
				ctx->Coros_.erase (thisPos);
		}

		bool HasDeadContexts () const
		{
			return !DeadContexts_.isEmpty ();
		}

		AddContext await_transform (AddContext awaitable) const
		{
			return awaitable;
		}

		template<typename T>
		auto await_transform (T&& awaitable)
		{
			using OrigAwaiter = decltype (detail::Awaiter (std::forward<T> (awaitable)));
			return detail::AwaitableWrapper<ContextExtensionBase, OrigAwaiter> { *this, detail::Awaiter (std::forward<T> (awaitable)) };
		}
	};

	template<typename>
	struct ContextExtension : ContextExtensionBase
	{
		using HasContextExtension = void;
	};
}
