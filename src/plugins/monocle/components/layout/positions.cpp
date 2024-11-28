/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "positions.h"
#include <QGraphicsView>
#include "components/viewitems/pagegraphicsitem.h"

namespace LC::Monocle
{
	PageRelativePos::PageRelativePos (PageRelativePosBase base)
	: PageRelativePosBase { base }
	{
	}

	PageAbsolutePos::PageAbsolutePos (PageAbsolutePosBase base)
	: PageAbsolutePosBase { base }
	{
	}

	PageAbsolutePos PageRelativePos::ToPageAbsolute (const PageGraphicsItem& item) const
	{
		return PageRelativePosBase::ToPageAbsolute (item.boundingRect ().size ());
	}

	SceneAbsolutePos PageRelativePos::ToSceneAbsolute (const PageGraphicsItem& item) const
	{
		return ToPageAbsolute (item).ToSceneAbsolute (item);
	}

	PageRelativePos PageAbsolutePos::ToPageRelative (const PageGraphicsItem& item) const
	{
		return PageAbsolutePosBase::ToPageRelative (item.boundingRect ().size ());
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

	SceneAbsolutePos ViewAbsolutePos::ToSceneAbsolute (const QGraphicsView& view) const
	{
		return SceneAbsolutePos { view.mapToScene (P_.toPoint ()) };
	}

	PageRelativeRect::PageRelativeRect (const PageRelativeRectBase& base)
	: PageRelativeRectBase { base }
	{
	}

	PageAbsoluteRect::PageAbsoluteRect (const PageAbsoluteRectBase& base)
	: PageAbsoluteRectBase { base }
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

	PageAbsoluteRect SceneAbsoluteRect::ToPageAbsolute (const PageGraphicsItem& item) const
	{
		return Convert<PageAbsoluteRect> (&SceneAbsolutePos::ToPageAbsolute, *this, item);
	}

	PageRelativeRect SceneAbsoluteRect::ToPageRelative (const PageGraphicsItem& item) const
	{
		return Convert<PageRelativeRect> (&SceneAbsolutePos::ToPageRelative, *this, item);
	}

	ViewAbsoluteRect::ViewAbsoluteRect (const QGraphicsView& view)
	: Rect { view.viewport ()->rect () }
	{
	}

	SceneAbsoluteRect ViewAbsoluteRect::ToSceneAbsolute (const QGraphicsView& view) const
	{
		return SceneAbsoluteRect { view.mapToScene (R_.toRect ()).boundingRect () };
	}
}
