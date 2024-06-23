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
	class PageGraphicsItem;

	struct PageRelativePos;
	struct PageAbsolutePos;
	struct SceneAbsolutePos;

	template<typename T>
	struct Pos
	{
		QPointF P_ {};

		[[nodiscard]]
		T ClearedX () const
		{
			return { QPointF { 0, P_.y () } };
		}

		[[nodiscard]]
		T ClearedY () const
		{
			return { QPointF { P_.x (), 0 } };
		}

		[[nodiscard]]
		T Shifted (qreal dx, qreal dy) const
		{
			return { P_ + QPointF { dx, dy } };
		}

		QPointF ToPointF () const
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

	struct PageRelativePos : Pos<PageRelativePos>
	{
		PageAbsolutePos ToPageAbsolute (const PageGraphicsItem&) const;
		SceneAbsolutePos ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct PageAbsolutePos : Pos<PageAbsolutePos>
	{
		PageRelativePos ToPageRelative (const PageGraphicsItem&) const;
		SceneAbsolutePos ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct SceneAbsolutePos : Pos<SceneAbsolutePos>
	{
		PageAbsolutePos ToPageAbsolute (const PageGraphicsItem&) const;
		PageRelativePos ToPageRelative (const PageGraphicsItem&) const;
	};
}
