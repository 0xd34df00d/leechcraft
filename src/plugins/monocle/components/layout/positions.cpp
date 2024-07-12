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
		return { PageAbsolutePos::Type { P_.x () * rect.width (), P_.y () * rect.height () } };
	}

	SceneAbsolutePos PageRelativePos::ToSceneAbsolute (const PageGraphicsItem& item) const
	{
		return ToPageAbsolute (item).ToSceneAbsolute (item);
	}

	PageRelativePos PageAbsolutePos::ToPageRelative (const PageGraphicsItem& item) const
	{
		const auto& rect = item.boundingRect ();
		return { PageAbsolutePos::Type { P_.x () / rect.width (), P_.y () / rect.height () } };
	}

	SceneAbsolutePos PageAbsolutePos::ToSceneAbsolute (const PageGraphicsItem& item) const
	{
		return { item.mapToScene (P_) };
	}

	PageAbsolutePos SceneAbsolutePos::ToPageAbsolute (const PageGraphicsItem& item) const
	{
		auto pagePos = item.mapFromScene (P_);
		const auto& itemSize = item.boundingRect ().size ();
		pagePos.rx () = std::clamp (pagePos.x (), 0.0, itemSize.width ());
		pagePos.ry () = std::clamp (pagePos.y (), 0.0, itemSize.height ());
		return { pagePos };
	}

	PageRelativePos SceneAbsolutePos::ToPageRelative (const PageGraphicsItem& item) const
	{
		return ToPageAbsolute (item).ToPageRelative (item);
	}
}
