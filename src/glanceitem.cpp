/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "glanceitem.h"
#include <QPropertyAnimation>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QtDebug>

namespace LeechCraft
{
	GlanceItem::GlanceItem (const QPixmap& px, QGraphicsItem *parent)
	: QGraphicsPixmapItem (px, parent)
	, Scale_ (0)
	, Anim_ (new QPropertyAnimation (this, "Scale"))
	{
		setAcceptHoverEvents (true);
		setTransformationMode (Qt::SmoothTransformation);
	}

	void GlanceItem::SetIndex (int idx)
	{
		Index_ = idx;
	}

	void GlanceItem::SetIdealScale (qreal scale)
	{
		Scale_ = scale;
	}

	void GlanceItem::QueueScaleAnim (qreal start, qreal end)
	{
		Anim_->stop ();
		Anim_->setDuration (500);
		Anim_->setStartValue (start);
		Anim_->setEndValue (end);

		Anim_->start ();
	}

	void GlanceItem::hoverEnterEvent (QGraphicsSceneHoverEvent*)
	{
		QueueScaleAnim (scale (), 0.5);
	}

	void GlanceItem::hoverLeaveEvent (QGraphicsSceneHoverEvent*)
	{
		QueueScaleAnim (scale (), Scale_);
	}

	void GlanceItem::mousePressEvent (QGraphicsSceneMouseEvent *e)
	{
		QGraphicsPixmapItem::mousePressEvent (e);
		e->accept ();
	}

	void GlanceItem::mouseReleaseEvent (QGraphicsSceneMouseEvent*)
	{
		emit clicked (Index_);
	}
};

