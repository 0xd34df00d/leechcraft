/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <memory>
#include <tuple>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <util/sll/either.h>
#include <util/sll/void.h>

namespace LC::Util::detail
{
	template<typename... Rets>
	struct DBusAwaiter
	{
		QDBusPendingReply<Rets...> Reply_;
		std::unique_ptr<QDBusPendingCallWatcher> Watcher_;

		DBusAwaiter (const QDBusPendingReply<Rets...>& reply)
		: Reply_ { reply }
		{
		}

		bool await_ready () const noexcept
		{
			return Reply_.isFinished ();
		}

		void await_suspend (std::coroutine_handle<> handle) noexcept
		{
			Watcher_ = std::make_unique<QDBusPendingCallWatcher> (Reply_);
			QObject::connect (Watcher_.get (),
					&QDBusPendingCallWatcher::finished,
					handle);
		}

		template<typename = void>
		struct SuccessType;

		template<typename T> requires (sizeof... (Rets) == 0)
		struct SuccessType<T> { using Type = Void; };

		template<typename T> requires (sizeof... (Rets) == 1)
		struct SuccessType<T> { using Type = Rets... [0]; };

		template<typename T> requires (sizeof... (Rets) > 1)
		struct SuccessType<T> { using Type = std::tuple<Rets...>; };

		using SuccessType_t = SuccessType<>::Type;

		Either<QDBusError, SuccessType_t> await_resume () const noexcept
		{
			if (Reply_.isError ())
				return { AsLeft, Reply_.error () };

			if constexpr (sizeof... (Rets) == 0)
				return { Void {} };
			else if constexpr (sizeof... (Rets) == 1)
				return { Reply_.value () };
			else
				return [&]<size_t... Idxs> (std::index_sequence<Idxs...>) -> std::tuple<Rets...>
				{
					return { Reply_.template argumentAt<Idxs> ()... };
				} (std::make_index_sequence<sizeof... (Rets)> {});
		}
	};
}

namespace LC
{
	template<typename... Rets>
	Util::detail::DBusAwaiter<Rets...> operator co_await (const QDBusPendingReply<Rets...>& reply)
	{
		return { reply };
	}
}

namespace LC::Util
{
	template<typename... Rets>
	detail::DBusAwaiter<Rets...> Typed (const QDBusPendingCall& asyncCall)
	{
		return { asyncCall };
	}
}
