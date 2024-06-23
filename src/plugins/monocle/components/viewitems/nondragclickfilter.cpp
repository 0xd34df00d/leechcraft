/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nondragclickfilter.h"
#include <QGraphicsSceneMouseEvent>

namespace LC::Monocle
{
	void NonDragClickFilter::RecordPressed (QGraphicsSceneMouseEvent *ev)
	{
		Pressed_ = ev->pos ();
	}

	bool NonDragClickFilter::IsNonDragRelease (QGraphicsSceneMouseEvent *ev) const
	{
		return (Pressed_ - ev->pos ()).manhattanLength () < 4;
	}
}
