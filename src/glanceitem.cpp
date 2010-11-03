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
	, ScaleAnim_ (new QPropertyAnimation (this, "Scale"))
	{
		setAcceptHoverEvents (true);
		setTransformationMode (Qt::SmoothTransformation);
		setCacheMode (ItemCoordinateCache);
		current = false;
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
		ScaleAnim_->stop ();
		ScaleAnim_->setDuration (500);
		ScaleAnim_->setStartValue (start);
		ScaleAnim_->setEndValue (end);

		ScaleAnim_->start ();
	}

	void GlanceItem::hoverEnterEvent (QGraphicsSceneHoverEvent*)
	{
		Q_FOREACH (GlanceItem* item, ItemsList_)
			if (item->isCurrent ())
				item->setCurrent (false);
		setCurrent (true);
	}

	void GlanceItem::hoverLeaveEvent (QGraphicsSceneHoverEvent*)
	{
		setCurrent (false);
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

	void GlanceItem::setCurrent (bool cur)
	{
		if (cur)
		{
			setZValue (1);
			QueueScaleAnim (scale (), 1);
		}
		else
		{
			setZValue (0);
			QueueScaleAnim (scale (), Scale_);
		}
		current = cur;
	}

	bool GlanceItem::isCurrent ()
	{
		if (current)
			return true;

		return false;
	}

	void GlanceItem::setItemList (QList<QGraphicsItem*> list)
	{
		Q_FOREACH (QGraphicsItem* item, list)
			ItemsList_ << qgraphicsitem_cast<GlanceItem*> (item);
	}
};

