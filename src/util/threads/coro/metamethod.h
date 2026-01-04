/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <QMetaObject>
#include <util/sll/void.h>

namespace LC::Util::detail
{
	template<typename Ctx, typename F, typename... Args>
	struct MethodAwaiter
	{
		Ctx& Ctx_;
		std::decay_t<F> Method_;
		std::tuple<Ctx&, std::decay_t<Args>...> Args_;

		using R = std::invoke_result_t<std::decay_t<F>, Ctx&, std::decay_t<Args>...>;
		constexpr static bool IsVoid = std::is_same_v<R, void>;

		std::conditional_t<IsVoid, Void, std::optional<R>> Result_ {};

		std::exception_ptr Exception_ {};

		std::atomic_bool Ready_ { false };

		bool await_ready () const noexcept
		{
			return false;
		}

		void await_suspend (std::coroutine_handle<> handle) noexcept
		{
			const auto thread = QThread::currentThread ();
			QMetaObject::invokeMethod (&Ctx_,
					[=, this]
					{
						try
						{
							if constexpr (IsVoid)
								std::apply (Method_, Args_);
							else
								Result_ = std::apply (Method_, Args_);
						}
						catch (...)
						{
							Exception_ = std::current_exception ();
						}

						Ready_.store (true, std::memory_order_release);

						QMetaObject::invokeMethod (thread, handle, Qt::QueuedConnection);
					},
					Qt::QueuedConnection);
		}

		R await_resume () const noexcept
		{
			while (!Ready_.load (std::memory_order_acquire))
				;

			if (Exception_)
				std::rethrow_exception (Exception_);

			if constexpr (!IsVoid)
				return *Result_;
		}
	};
}

namespace LC::Util
{
	template<typename Ctx, typename F, typename... Args>
	auto MetaMethod (Ctx& ctx, F&& method, Args&&... args)
	{
		return detail::MethodAwaiter<Ctx, F, Args...> { ctx, std::forward<F> (method), { ctx, std::forward<Args> (args)... } };
	}
}
