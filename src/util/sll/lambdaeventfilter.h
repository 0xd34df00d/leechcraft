/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>
#include <QEvent>
#include <QObject>
#include "typegetter.h"

namespace LC::Util
{
	namespace detail
	{
		template<QEvent::Type Type, typename F>
		class LambdaEventFilter : public QObject
		{
			const F F_;

			using EventType_t = std::remove_pointer_t<std::decay_t<ArgType_t<F, 0>>>;
		public:
			LambdaEventFilter (F&& f, QObject& parent)
			: QObject { &parent }
			, F_ { std::move (f) }
			{
			}

			bool eventFilter (QObject*, QEvent *srcEv) override
			{
				if (Type != QEvent::None && Type != srcEv->type ())
					return false;

				const auto ev = dynamic_cast<EventType_t*> (srcEv);
				if (!ev)
					return false;

				if constexpr (requires { F_ (ev, static_cast<QObject&> (*this)); })
					return F_ (ev, static_cast<QObject&> (*this));
				else
					return F_ (ev);
			}
		};
	}

	template<QEvent::Type Type = QEvent::None, typename F>
	auto MakeLambdaEventFilter (F&& f, QObject& parent)
	{
		return new detail::LambdaEventFilter<Type, std::decay_t<F>> { std::forward<F> (f), parent };
	}
}
