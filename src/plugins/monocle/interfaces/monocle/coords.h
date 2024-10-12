/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPointF>
#include <QRectF>

namespace LC::Monocle
{
	enum class Relativity
	{
		PageRelative,
		PageAbsolute,
		SceneAbsolute,
		ViewAbsolute,
	};

	template<typename T, Relativity R>
	struct Pos
	{
		static constexpr auto Relativity = R;

		using Type = QPointF;

		Type P_ {};

		Pos () = default;

		explicit Pos (Type p)
		: P_ { p }
		{
		}

		explicit Pos (qreal x, qreal y)
		: P_ { x, y }
		{
		}

		auto operator<=> (const Pos&) const = default;

		[[nodiscard]]
		T ClearedX () const
		{
			return T { 0, P_.y () };
		}

		[[nodiscard]]
		T ClearedY () const
		{
			return T { P_.x (), 0 };
		}

		[[nodiscard]]
		T Shifted (qreal dx, qreal dy) const
		{
			return T { P_ + Type { dx, dy } };
		}

		Type ToPointF () const
		{
			return P_;
		}

		friend T operator+ (T p1, T p2)
		{
			return T { p1.P_ + p2.P_ };
		}

		friend T operator- (T p1, T p2)
		{
			return T { p1.P_ - p2.P_ };
		}

		friend T operator* (T p, qreal factor)
		{
			return T { p.P_ * factor };
		}

		friend T operator/ (T p, qreal factor)
		{
			return T { p.P_ / factor };
		}

		bool BothGeqThan (T p) const
		{
			return P_.x () >= p.P_.x () && P_.y () >= p.P_.y ();
		}

		bool BothLeqThan (T p) const
		{
			return P_.x () <= p.P_.x () && P_.y () <= p.P_.y ();
		}
	};

	template<typename T, Relativity R>
	struct Rect
	{
		static constexpr auto Relativity = R;

		using Type = QRectF;

		Type R_ {};

		Rect () = default;

		explicit Rect (const Type& r)
		: R_ { r }
		{
		}

		template<typename K>
		explicit Rect (Pos<K, R> topLeft, Pos<K, R> bottomRight)
		: R_ { topLeft.ToPointF (), bottomRight.ToPointF () }
		{
		}

		auto operator<=> (const Rect&) const = default;

		template<typename P>
			requires (P::Relativity == R)
		P TopLeft () const
		{
			return P { R_.topLeft () };
		}

		template<typename P>
			requires (P::Relativity == R)
		P BottomRight () const
		{
			return P { R_.bottomRight () };
		}

		Type ToRectF () const
		{
			return R_;
		}

		bool IsEmpty () const
		{
			return R_.isEmpty ();
		}

		friend T operator| (const T& r1, const T& r2)
		{
			return T { r1.R_ | r2.R_ };
		}

		friend T operator& (const T& r1, const T& r2)
		{
			return T { r1.R_ & r2.R_ };
		}
	};

	struct PageRelativeRectBase : Rect<PageRelativeRectBase, Relativity::PageRelative>
	{
		using Rect::Rect;
	};
}
