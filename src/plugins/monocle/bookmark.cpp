/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmark.h"
#include "interfaces/monocle/ilink.h"

namespace LC::Monocle
{
	NavigationAction Bookmark::ToNavigationAction () const
	{
		const QRectF targetArea { Position_, QSizeF { 1, 1 } };
		return { .PageNumber_ = Page_, .TargetArea_ = targetArea };
	}

	bool operator== (const Bookmark& b1, const Bookmark& b2)
	{
		return b1.Position_ == b2.Position_ && b1.Name_ == b2.Name_;
	}
}
