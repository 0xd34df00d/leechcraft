/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QPointF>
#include <QVector>
#include "positions.h"

namespace LC::Monocle
{
	class PageGraphicsItem;

	class PositionTracker
	{
		struct BottomRightCorner
		{
			double Y_;
			double X_;

			explicit BottomRightCorner (SceneAbsolutePos);

			auto operator<=> (const BottomRightCorner&) const = default;
		};

		const QVector<PageGraphicsItem*>& Pages_;
		QMap<BottomRightCorner, int> Corner2PageInfo_;
	public:
		explicit PositionTracker (const QVector<PageGraphicsItem*>&);

		void Update ();
		int GetNearbyPage (SceneAbsolutePos) const;
	};
}
