/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmark.h"
#include <QDebug>
#include "interfaces/monocle/ilink.h"

namespace LC::Monocle
{
	NavigationAction Bookmark::ToNavigationAction () const
	{
		return { .PageNumber_ = Page_, .TargetArea_ = PageRelativeRectBase { Position_, Position_ } };
	}

	QDebug operator<< (QDebug dbg, const Bookmark& bm)
	{
		const QDebugStateSaver saver { dbg };
		dbg.nospace () << "{ page: " << bm.Page_ << "; pos: " << bm.Position_.ToPointF () << "; name: " << bm.Name_ << " }";
		return dbg;
	}
}
