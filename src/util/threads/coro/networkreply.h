/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <variant>
#include "threadsconfig.h"

class QNetworkReply;

namespace LC::Util
{
	class NetworkResult;

	namespace detail
	{
		struct UTIL_THREADS_API NRAwaiter
		{
			QNetworkReply& Reply_;

			bool await_ready () const noexcept;
			void await_suspend (std::coroutine_handle<> handle) noexcept;
			NetworkResult await_resume () const noexcept;
		};
	}

	UTIL_THREADS_API detail::NRAwaiter operator co_await (QNetworkReply& reply);
}
