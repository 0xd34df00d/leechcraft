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

#include "graphwidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPolygonF>
#include <QtDebug>

using namespace LeechCraft::Util;

GraphWidget::GraphWidget (const QColor& downColor,
		const QColor& upColor,
		QWidget *parent)
: QFrame (parent)
, DownColor_ (downColor)
, UpColor_ (upColor)
{
	setFrameShape (QFrame::Box);
	setAutoFillBackground (true);
	setBackgroundRole (QPalette::Window);
	setPalette (Qt::transparent);
	PushSpeed (0, 0);
}

void GraphWidget::PushSpeed (quint64 ds, quint64 us)
{
	if (DownSpeeds_.size () == width ())
	{
		DownSpeeds_.removeFirst ();
		UpSpeeds_.removeFirst ();
	}
	DownSpeeds_.append (ds);
	UpSpeeds_.append (us);
	update ();
}

void GraphWidget::paintEvent (QPaintEvent*)
{
	quint64 max = 0;

	for (QList <quint64>::const_iterator dsIt = DownSpeeds_.begin (), dsEnd = DownSpeeds_.end (),
		 usIt = UpSpeeds_.begin (), usEnd = UpSpeeds_.end ();
		 dsIt != dsEnd && usIt != usEnd;
		 ++dsIt, ++usIt)
	{
		max = qMax (max, qMax (*dsIt, *usIt));
	}

	QPainter painter (this);
	painter.setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	if (max && DownSpeeds_.size ())
	{
		painter.setPen (UpColor_);
		PaintSingle (max, UpSpeeds_, &painter);
		painter.setPen (DownColor_);
		PaintSingle (max, DownSpeeds_, &painter);
	}
}

void GraphWidget::PaintSingle (quint64 max, const QList<quint64>& speeds,
		QPainter *painter)
{
	const int prevX = width () - 1;
	int i = speeds.size () - 1;
	const double prevY = height () * (1 - static_cast<double> (speeds.at (i)) / static_cast<double> (max));

	for ( ; i >= 0; --i)
	{
		const int x = width () - speeds.size () + i;
		const double y = height () * (1 - static_cast<double> (speeds.at (i)) / static_cast<double> (max));
		painter->drawLine (QPointF (prevX, prevY), QPointF (x, y));
	}
}

