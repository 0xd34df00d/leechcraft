/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPointF>

namespace LC::Monocle
{
	template<typename T>
	struct Pos
	{
		using Type = QPointF;

		Type P_ {};

		auto operator<=> (const Pos&) const = default;

		[[nodiscard]]
		T ClearedX () const
		{
			return { Type { 0, P_.y () } };
		}

		[[nodiscard]]
		T ClearedY () const
		{
			return { Type { P_.x (), 0 } };
		}

		[[nodiscard]]
		T Shifted (qreal dx, qreal dy) const
		{
			return { P_ + Type { dx, dy } };
		}

		Type ToPointF () const
		{
			return P_;
		}

		friend T operator+ (T p1, T p2)
		{
			return { p1.P_ + p2.P_ };
		}

		friend T operator- (T p1, T p2)
		{
			return { p1.P_ - p2.P_ };
		}

		friend T operator* (T p, qreal factor)
		{
			return { p.P_ * factor };
		}

		friend T operator/ (T p, qreal factor)
		{
			return { p.P_ / factor };
		}
	};

	struct PageRelativePosBase : Pos<PageRelativePosBase> {};
}
