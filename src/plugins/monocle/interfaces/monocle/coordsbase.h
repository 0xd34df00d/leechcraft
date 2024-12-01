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

	template<Relativity R>
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

		template<typename Self>
		[[nodiscard]]
		auto ClearedX (this const Self& self)
		{
			return Self { 0, self.P_.y () };
		}

		template<typename Self>
		[[nodiscard]]
		auto ClearedY (this const Self& self)
		{
			return Self { self.P_.x (), 0 };
		}

		template<typename Self>
		[[nodiscard]]
		Self Shifted (this const Self& self, qreal dx, qreal dy)
		{
			return Self { self.P_ + Type { dx, dy } };
		}

		Type ToPointF () const
		{
			return P_;
		}

		template<typename Self>
		Self operator+ (this Self p1, Self p2)
		{
			return Self { p1.P_ + p2.P_ };
		}

		template<typename Self>
		Self operator- (this Self p1, Self p2)
		{
			return Self { p1.P_ - p2.P_ };
		}

		template<typename Self>
		Self operator* (this Self p, qreal factor)
		{
			return Self { p.P_ * factor };
		}

		template<typename Self>
		Self operator/ (this Self p, qreal factor)
		{
			return Self { p.P_ / factor };
		}

		bool BothGeqThan (Pos p) const
		{
			return P_.x () >= p.P_.x () && P_.y () >= p.P_.y ();
		}

		bool BothLeqThan (Pos p) const
		{
			return P_.x () <= p.P_.x () && P_.y () <= p.P_.y ();
		}
	};

	template<Relativity R>
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

		explicit Rect (Pos<R> topLeft, Pos<R> bottomRight)
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

		void SetLeft (qreal x)
		{
			R_.setLeft (x);
		}

		void SetRight (qreal x)
		{
			R_.setRight (x);
		}

		Type ToRectF () const
		{
			return R_;
		}

		bool IsEmpty () const
		{
			return R_.isEmpty ();
		}

		template<typename Self>
		Self operator| (this const Self& r1, const Self& r2)
		{
			return Self { r1.R_ | r2.R_ };
		}

		template<typename Self>
		Self operator& (this const Self& r1, const Self& r2)
		{
			return Self { r1.R_ & r2.R_ };
		}
	};

	struct PageRelativePosBase;
	struct PageAbsolutePosBase;

	struct PageRelativePosBase : Pos<Relativity::PageRelative>
	{
		using Pos::Pos;

		PageAbsolutePosBase ToPageAbsolute (QSizeF) const;
	};

	struct PageAbsolutePosBase : Pos<Relativity::PageAbsolute>
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

	struct PageRelativeRectBase : Rect<Relativity::PageRelative>
	{
		using Rect::Rect;

		PageAbsoluteRectBase ToPageAbsolute (QSizeF) const;
	};

	struct PageAbsoluteRectBase : Rect<Relativity::PageAbsolute>
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
