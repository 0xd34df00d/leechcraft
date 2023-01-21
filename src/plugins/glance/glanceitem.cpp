/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glanceitem.h"
#include <QPropertyAnimation>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QIcon>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>

namespace LC
{
namespace Plugins
{
namespace Glance
{
	GlanceItem::GlanceItem (const QPixmap& px, const QRect& closeButtonRect, QGraphicsItem *parent)
	: QGraphicsPixmapItem (px, parent)
	, ScaleAnim_ (new QPropertyAnimation (this, "Scale"))
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

	void GlanceItem::hoverEnterEvent (QGraphicsSceneHoverEvent*)
	{
		for (const auto item : ItemsList_)
			if (item->IsCurrent () && item != this)
				item->SetCurrent (false);
		SetCurrent (true);
	}

	void GlanceItem::hoverMoveEvent (QGraphicsSceneHoverEvent *e)
	{
		if (CloseButtonRect_.contains (e->pos ().toPoint ()))
			DrawCloseButton (true);
		else
			DrawCloseButton (false);
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
			QueueScaleAnim (scale (), std::max (0.5, Scale_ * 1.2));
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
		ItemsList_ = Util::Map (list, [] (auto item) { return qgraphicsitem_cast<GlanceItem*> (item); });
	}

	void GlanceItem::DrawCloseButton (bool selected)
	{
		QPixmap px (Pixmap_);
		QPainter p (&px);

		const auto& closeIcon = GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("window-close");
		closeIcon.paint (&p, CloseButtonRect_, Qt::AlignCenter, selected ? QIcon::Selected : QIcon::Normal);

		p.end ();
		setPixmap (px);
	}
};
};
};
