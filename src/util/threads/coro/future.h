/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <QFuture>
#include <QFutureWatcher>

namespace LC::Util::detail
{
	template<typename R>
	struct FutureAwaiter
	{
		QFuture<R> Future_;

		bool await_ready () const noexcept
		{
			return Future_.isFinished ();
		}

		void await_suspend (std::coroutine_handle<> cont) noexcept
		{
			auto watcher = new QFutureWatcher<R> {};
			watcher->setFuture (Future_);
			QObject::connect (watcher,
					&QFutureWatcher<R>::finished,
					[=]
					{
						watcher->deleteLater ();
						cont ();
					});
		}

		R await_resume () const noexcept
		{
			if constexpr (!std::is_same_v<R, void>)
				return Future_.result ();
		}
	};
}

namespace LC
{
	template<typename R>
	Util::detail::FutureAwaiter<R> operator co_await (QFuture<R> future)
	{
		return { std::move (future) };
	}
}
