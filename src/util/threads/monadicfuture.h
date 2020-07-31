/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QFuture>
#include <util/sll/functor.h>
#include <util/sll/monad.h>
#include "futures.h"

namespace LC
{
namespace Util
{
	template<typename T>
	struct InstanceFunctor<QFuture<T>>
	{
		template<typename F>
		using FmapResult_t = QFuture<std::decay_t<std::result_of_t<F (T)>>>;

		template<typename F>
		static FmapResult_t<F> Apply (const QFuture<T>& fut, const F& func)
		{
			return Sequence (nullptr, fut) >>
					[func] (const T& val) { return MakeReadyFuture (func (val)); };
		}
	};
}
}
