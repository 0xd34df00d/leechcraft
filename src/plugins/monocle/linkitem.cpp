/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linkitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QPen>

namespace LC
{
namespace Monocle
{
	LinkItem::LinkItem (const ILink_ptr& link, QGraphicsItem *parent)
	: QGraphicsRectItem { parent }
	, Link_ { link }
	{
		setCursor (Qt::PointingHandCursor);
		setPen (Qt::NoPen);
		setFlag (QGraphicsItem::ItemHasNoContents);
	}

	void LinkItem::mousePressEvent (QGraphicsSceneMouseEvent *event)
	{
		PressedPos_ = event->pos ();
	}

	void LinkItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
	{
		if ((event->pos () - PressedPos_).manhattanLength () < 4)
			Link_->Execute ();
	}
}
}
