/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QPainter>
#include <QIcon>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "core.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Glance
{
	GlanceItem::GlanceItem (const QPixmap& px, const QRect& closeButtonRect, QGraphicsItem *parent)
	: QGraphicsPixmapItem (px, parent)
	, Scale_ (0)
	, ScaleAnim_ (new QPropertyAnimation (this, "Scale"))
	, Current_ (false)
	, CloseButtonRect_ (closeButtonRect)
	, Pixmap_ (px)
	{
		setAcceptHoverEvents (true);
		setTransformationMode (Qt::SmoothTransformation);
		setCacheMode (ItemCoordinateCache);

		DrawCloseButton(false);
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
		ScaleAnim_->setDuration (300);
		ScaleAnim_->setStartValue (start);
		ScaleAnim_->setEndValue (end);

		ScaleAnim_->start ();
	}

	void GlanceItem::hoverEnterEvent (QGraphicsSceneHoverEvent *e)
	{
		Q_FOREACH (GlanceItem* item, ItemsList_)
			if (item->IsCurrent () && item != this)
				item->SetCurrent (false);
		SetCurrent (true);
	}

	void GlanceItem::hoverMoveEvent (QGraphicsSceneHoverEvent *e)
	{
		if (CloseButtonRect_.contains (e->pos ().toPoint ()))
				DrawCloseButton(true);
		else
				DrawCloseButton(false);
	}

	void GlanceItem::hoverLeaveEvent (QGraphicsSceneHoverEvent*)
	{
		SetCurrent (false);
		DrawCloseButton (false);
	}

	void GlanceItem::mousePressEvent (QGraphicsSceneMouseEvent *e)
	{
		QGraphicsPixmapItem::mousePressEvent (e);
		e->accept ();
	}

	void GlanceItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *e)
	{
		const auto& clickPoint = e->buttonDownPos (Qt::LeftButton).toPoint ();
		if (CloseButtonRect_.contains (clickPoint))
			emit clicked (Index_, true);
		else
			emit clicked (Index_, false);
	}

	void GlanceItem::SetCurrent (bool cur)
	{
		if (cur)
		{
			setZValue (1);
			QueueScaleAnim (scale (), std::max (0.5, Scale_ * 1.3));
		}
		else
		{
			setZValue (0);
			QueueScaleAnim (scale (), Scale_);
		}
		Current_ = cur;
	}

	bool GlanceItem::IsCurrent () const
	{
		return Current_;
	}

	void GlanceItem::SetItemList (QList<QGraphicsItem*> list)
	{
		Q_FOREACH (QGraphicsItem* item, list)
			ItemsList_ << qgraphicsitem_cast<GlanceItem*> (item);
	}

	void GlanceItem::DrawCloseButton (bool selected)
	{
		QPixmap px (Pixmap_);
		QPainter p (&px);

		QIcon closeIcon = Core::Instance ().GetProxy ()->GetIcon ("window-close");
		closeIcon.paint (&p, CloseButtonRect_, Qt::AlignCenter, selected ? QIcon::Selected : QIcon::Normal);

		p.end ();
		setPixmap (px);
	}
};
};
};
