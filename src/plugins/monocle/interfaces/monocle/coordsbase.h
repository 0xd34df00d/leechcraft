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
	enum class Relativity : std::uint8_t
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

	struct PageRelativePosBase;
	struct PageAbsolutePosBase;

	struct PageRelativePosBase : Pos<PageRelativePosBase, Relativity::PageRelative>
	{
		using Pos::Pos;

		PageAbsolutePosBase ToPageAbsolute (QSizeF) const;
	};

	struct PageAbsolutePosBase : Pos<PageAbsolutePosBase, Relativity::PageAbsolute>
	{
		using Pos::Pos;

		PageRelativePosBase ToPageRelative (QSizeF) const;
	};

	inline PageAbsolutePosBase PageRelativePosBase::ToPageAbsolute (QSizeF size) const
	{
		return PageAbsolutePosBase { P_.x () * size.width (), P_.y () * size.height () };
	}

	inline PageRelativePosBase PageAbsolutePosBase::ToPageRelative (QSizeF size) const
	{
		return PageRelativePosBase { P_.x () / size.width (), P_.y () / size.height () };
	}

	struct PageRelativeRectBase;
	struct PageAbsoluteRectBase;

	struct PageRelativeRectBase : Rect<PageRelativeRectBase, Relativity::PageRelative>
	{
		using Rect::Rect;

		PageAbsoluteRectBase ToPageAbsolute (QSizeF) const;
	};

	struct PageAbsoluteRectBase : Rect<PageAbsolutePosBase, Relativity::PageAbsolute>
	{
		using Rect::Rect;

		PageRelativeRectBase ToPageRelative (QSizeF) const;
	};

	namespace detail
	{
		template<typename Target, typename SrcPos, typename TargetPos, typename Source, typename Ctx>
		Target Convert (TargetPos (SrcPos::*posConvert) (Ctx) const, const Source& src, const auto& context)
		{
			return Target
			{
				(src.template TopLeft<SrcPos> ().*posConvert) (context),
				(src.template BottomRight<SrcPos> ().*posConvert) (context)
			};
		}
	}

	inline PageAbsoluteRectBase PageRelativeRectBase::ToPageAbsolute (QSizeF size) const
	{
		return detail::Convert<PageAbsoluteRectBase> (&PageRelativePosBase::ToPageAbsolute, *this, size);
	}

	inline PageRelativeRectBase PageAbsoluteRectBase::ToPageRelative (QSizeF size) const
	{
		return detail::Convert<PageRelativeRectBase> (&PageAbsolutePosBase::ToPageRelative, *this, size);
	}
}
