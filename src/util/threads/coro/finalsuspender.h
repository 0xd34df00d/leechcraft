/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>

namespace LC::Util::detail
{
	template<typename Promise>
	struct FinalSuspender
	{
		Promise& Promise_;

		bool await_ready () const noexcept { return false; }

		void await_suspend (std::coroutine_handle<>) noexcept
		{
			decltype (Promise_.WaitingHandles_) handles;
			{
				std::lock_guard lock { Promise_ };
				handles = Promise_.WaitingHandles_;
			}

			for (auto h : handles)
				h ();

			Promise_.DecRef ();
		}

		void await_resume () const noexcept {}
	};
}
