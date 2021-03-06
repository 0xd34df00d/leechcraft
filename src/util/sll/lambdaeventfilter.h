/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>
#include <QObject>
#include "typegetter.h"

namespace LC
{
namespace Util
{
	namespace detail
	{
		template<typename F>
		class LambdaEventFilter : public QObject
		{
			const F F_;

			using EventType_t = std::remove_pointer_t<std::decay_t<ArgType_t<F, 0>>>;
		public:
			LambdaEventFilter (F&& f, QObject *parent = nullptr)
			: QObject { parent }
			, F_ { std::move (f) }
			{
			}

			bool eventFilter (QObject*, QEvent *srcEv) override
			{
				const auto ev = dynamic_cast<EventType_t*> (srcEv);
				if (!ev)
					return false;

				return F_ (ev);
			}
		};
	}

	template<typename F>
	auto MakeLambdaEventFilter (F&& f, QObject *parent = nullptr)
	{
		return new detail::LambdaEventFilter<std::decay_t<F>> { std::forward<F> (f), parent };
	}
}
}
