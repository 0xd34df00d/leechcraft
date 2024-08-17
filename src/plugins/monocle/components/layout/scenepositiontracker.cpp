/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scenepositiontracker.h"
#include <QtDebug>
#include "components/viewitems/pagegraphicsitem.h"

namespace LC::Monocle
{
	ScenePositionTracker::BottomRightCorner::BottomRightCorner (SceneAbsolutePos p)
	: Y_ { p.ToPointF ().y () }
	, X_ { p.ToPointF ().x () }
	{
	}

	ScenePositionTracker::ScenePositionTracker (const QVector<PageGraphicsItem*>& pages)
	: Pages_ { pages }
	{
	}

	void ScenePositionTracker::Clear ()
	{
		Corner2PageInfo_.clear ();
	}

	void ScenePositionTracker::Update ()
	{
		Corner2PageInfo_.clear ();

		for (int i = 0, size = Pages_.size (); i < size; ++i)
		{
			const auto page = Pages_.at (i);
			const auto sceneCornerPos = PageAbsolutePos { page->boundingRect ().bottomRight () }.ToSceneAbsolute (*page);
			Corner2PageInfo_ [BottomRightCorner { sceneCornerPos }] = i;
		}
	}

	int ScenePositionTracker::GetNearbyPage (SceneAbsolutePos pos) const
	{
		if (Corner2PageInfo_.isEmpty ())
			return -1;

		const auto infoPos = Corner2PageInfo_.lowerBound (BottomRightCorner { pos });
		return infoPos != Corner2PageInfo_.end () ?
				*infoPos :
				Pages_.size () - 1;
	}
}
