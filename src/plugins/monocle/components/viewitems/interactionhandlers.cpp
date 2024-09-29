/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "interactionhandlers.h"
#include "pagesview.h"

namespace LC::Monocle
{
	MovingInteraction::MovingInteraction (PagesView& view)
	{
		view.SetShowReleaseMenu (false);
		view.setDragMode (QGraphicsView::ScrollHandDrag);
	}

	AreaSelectionInteraction::AreaSelectionInteraction (PagesView& view)
	{
		view.SetShowReleaseMenu (true);
		view.setDragMode (QGraphicsView::RubberBandDrag);
	}
}
