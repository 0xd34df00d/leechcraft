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

		std::coroutine_handle<> await_suspend (std::coroutine_handle<>) noexcept
		{
			auto handles = [this]
			{
				std::lock_guard guard { Promise_ };
				return Promise_.GetAwaiters ();
			} ();
			Promise_.DecRef ();

			if constexpr (std::is_same_v<decltype (handles), std::coroutine_handle<>>)
				return handles ? handles : std::noop_coroutine ();
			else
			{
				if (handles.size () == 1)
					return handles [0];

				for (const auto& h : handles)
					h.resume ();
				return std::noop_coroutine ();
			}
		}

		void await_resume () const noexcept {}
	};
}
