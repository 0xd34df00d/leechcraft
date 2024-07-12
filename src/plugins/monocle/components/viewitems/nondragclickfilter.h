/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "components/layout/positions.h"

class QGraphicsSceneMouseEvent;

namespace LC::Monocle
{
	class NonDragClickFilter
	{
		SceneAbsolutePos Pressed_;
	public:
		void RecordPressed (QGraphicsSceneMouseEvent*);
		bool IsNonDragRelease (QGraphicsSceneMouseEvent*) const;
	};
}
