/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "positions.h"
#include "pagegraphicsitem.h"

namespace LC::Monocle
{
	PageAbsolutePos PageRelativePos::ToPageAbsolute (const PageGraphicsItem& item) const
	{
		const auto& rect = item.boundingRect ();
		return PageAbsolutePos { P_.x () * rect.width (), P_.y () * rect.height () };
	}

	SceneAbsolutePos PageRelativePos::ToSceneAbsolute (const PageGraphicsItem& item) const
	{
		return ToPageAbsolute (item).ToSceneAbsolute (item);
	}

	PageRelativePos PageAbsolutePos::ToPageRelative (const PageGraphicsItem& item) const
	{
		const auto& rect = item.boundingRect ();
		return PageRelativePos { P_.x () / rect.width (), P_.y () / rect.height () };
	}

	SceneAbsolutePos PageAbsolutePos::ToSceneAbsolute (const PageGraphicsItem& item) const
	{
		return SceneAbsolutePos { item.mapToScene (P_) };
	}

	PageAbsolutePos SceneAbsolutePos::ToPageAbsolute (const PageGraphicsItem& item) const
	{
		auto pagePos = item.mapFromScene (P_);
		const auto& itemSize = item.boundingRect ().size ();
		pagePos.rx () = std::clamp (pagePos.x (), 0.0, itemSize.width ());
		pagePos.ry () = std::clamp (pagePos.y (), 0.0, itemSize.height ());
		return PageAbsolutePos { pagePos };
	}

	PageRelativePos SceneAbsolutePos::ToPageRelative (const PageGraphicsItem& item) const
	{
		return ToPageAbsolute (item).ToPageRelative (item);
	}

	PageRelativeRect::PageRelativeRect (const PageRelativeRectBase& rb)
	: PageRelativeRectBase { rb }
	{
	}

	namespace
	{
		template<typename Target, typename SrcPos, typename TargetPos, typename Source>
		Target Convert (TargetPos (SrcPos::*posConvert) (const PageGraphicsItem&) const, const Source& src, const PageGraphicsItem& item)
		{
			return Target { (src.template TopLeft<SrcPos> ().*posConvert) (item), (src.template BottomRight<SrcPos> ().*posConvert) (item) };
		}
	}

	PageAbsoluteRect PageRelativeRect::ToPageAbsolute (const PageGraphicsItem& item) const
	{
		return Convert<PageAbsoluteRect> (&PageRelativePos::ToPageAbsolute, *this, item);
	}

	SceneAbsoluteRect PageRelativeRect::ToSceneAbsolute (const PageGraphicsItem& item) const
	{
		return Convert<SceneAbsoluteRect> (&PageRelativePos::ToSceneAbsolute, *this, item);
	}

	PageRelativeRect PageAbsoluteRect::ToPageRelative (const PageGraphicsItem& item) const
	{
		return Convert<PageRelativeRect> (&PageAbsolutePos::ToPageRelative, *this, item);
	}

	SceneAbsoluteRect PageAbsoluteRect::ToSceneAbsolute (const PageGraphicsItem& item) const
	{
		return Convert<SceneAbsoluteRect> (&PageAbsolutePos::ToSceneAbsolute, *this, item);
	}

	PageRelativeRect PageAbsoluteRect::ToPageRelative (QSizeF pageSize) const
	{
		auto rect = R_;
		rect.moveTop (rect.top () / pageSize.height ());
		rect.setHeight (rect.height () / pageSize.height ());
		rect.moveLeft (rect.left () / pageSize.width ());
		rect.setWidth (rect.width () / pageSize.width ());
		return PageRelativeRect { rect };
	}

	PageAbsoluteRect SceneAbsoluteRect::ToPageAbsolute (const PageGraphicsItem& item) const
	{
		return Convert<PageAbsoluteRect> (&SceneAbsolutePos::ToPageAbsolute, *this, item);
	}

	PageRelativeRect SceneAbsoluteRect::ToPageRelative (const PageGraphicsItem& item) const
	{
		return Convert<PageRelativeRect> (&SceneAbsolutePos::ToPageRelative, *this, item);
	}
}
