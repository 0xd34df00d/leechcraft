/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glanceitem.h"
#include <QApplication>
#include <QStyle>
#include <QStyleOption>
#include <QPropertyAnimation>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QIcon>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>

namespace LC::Glance
{
	namespace
	{
		QRect CalcCloseButtonRect (const QSize& pxSize)
		{
			const auto buttonSize = 24;
			const QPoint topLeft { 0, pxSize.width () - buttonSize };
			return { topLeft, QSize { buttonSize, buttonSize } };
		}
	}

	GlanceItem::GlanceItem (const QPixmap& px, QGraphicsItem *parent)
	: QGraphicsPixmapItem { px, parent }
	, ScaleAnim_ { new QPropertyAnimation { this, "Scale" } }
	, CloseButtonRect_ { CalcCloseButtonRect (px.size ()) }
	, Pixmap_ { px }
	{
		setAcceptHoverEvents (true);
		setTransformationMode (Qt::SmoothTransformation);
		setCacheMode (ItemCoordinateCache);

		DrawCloseButton (false);
	}

	void GlanceItem::SetIndex (int idx)
	{
		Index_ = idx;
	}

	void GlanceItem::SetIdealScale (qreal scale)
	{
		Scale_ = scale;
		QueueScaleAnim ();
	}

	namespace
	{
		auto GetTargetScale (bool current, qreal scale)
		{
			constexpr auto minScale = 0.5;
			constexpr auto scaleIncreaseFactor = 1.2;

			return current ?
					std::max (minScale, scale * scaleIncreaseFactor) :
					scale;
		}
	}

	void GlanceItem::QueueScaleAnim ()
	{
		ScaleAnim_->stop ();
		ScaleAnim_->setDuration (400);
		ScaleAnim_->setStartValue (scale ());
		ScaleAnim_->setEndValue (GetTargetScale (Current_, Scale_));

		ScaleAnim_->start ();
	}

	void GlanceItem::hoverEnterEvent (QGraphicsSceneHoverEvent*)
	{
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
			emit tabClosed (Index_);
		else
			emit tabSelected (Index_);
	}

	void GlanceItem::SetCurrent (bool cur)
	{
		Current_ = cur;
		setZValue (cur ? 1 : 0);
		QueueScaleAnim ();
	}

	bool GlanceItem::IsCurrent () const
	{
		return Current_;
	}

	void GlanceItem::DrawCloseButton (bool selected)
	{
		auto px = Pixmap_;
		QPainter p { &px };

		const auto& closeIcon = GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("window-close"_qs);
		closeIcon.paint (&p, CloseButtonRect_, Qt::AlignCenter, selected ? QIcon::Selected : QIcon::Normal);

		p.end ();
		setPixmap (px);
	}
}
