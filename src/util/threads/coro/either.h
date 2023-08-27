/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/either.h>

namespace LC::Util
{
	template<typename L, std::default_initializable R>
	struct IgnoreLeft
	{
		Either<L, R> Result_;

		bool await_ready () const noexcept
		{
			return true;
		}

		void await_suspend (auto) const noexcept
		{
		}

		R await_resume () const noexcept
		{
			return RightOr (Result_, R {});
		}
	};

	template<typename L, typename R>
	IgnoreLeft (Either<L, R>) -> IgnoreLeft<L, R>;
}
