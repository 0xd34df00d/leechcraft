/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsRectItem>
#include "interfaces/monocle/ilink.h"

namespace LC
{
namespace Monocle
{
	class DocumentTab;

	class LinkItem : public QGraphicsRectItem
	{
		DocumentTab& DocTab_;
		const ILink_ptr Link_;

		QPointF PressedPos_;
	public:
		LinkItem (const ILink_ptr&, QGraphicsItem*, DocumentTab&);
	protected:
		void mousePressEvent (QGraphicsSceneMouseEvent*);
		void mouseReleaseEvent (QGraphicsSceneMouseEvent*);
	};
}
}
