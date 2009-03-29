#include "graphwidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPolygonF>
#include <QtDebug>

using namespace LeechCraft::Util;

GraphWidget::GraphWidget (const QColor& downColor,
		const QColor& upColor,
		QWidget *parent)
: QWidget (parent)
, DownColor_ (downColor)
, UpColor_ (upColor)
{
    setAutoFillBackground (true);
    setBackgroundRole (QPalette::Window);
    setPalette (Qt::black);
}

void GraphWidget::PushSpeed (quint64 ds, quint64 us)
{
	if (DownSpeeds_.size () == width ())
	{
		DownSpeeds_.removeAt (0);
		UpSpeeds_.removeAt (0);
	}
	DownSpeeds_.append (ds);
	UpSpeeds_.append (us);
	update ();
}

void GraphWidget::paintEvent (QPaintEvent*)
{
    quint64 max = 0;
    for (int i = 0; i < DownSpeeds_.size (); ++i)
	{
        if (DownSpeeds_.at (i) > max)
            max = DownSpeeds_.at (i);
		if (UpSpeeds_.at (i) > max)
			max = UpSpeeds_.at (i);
	}

    QPainter painter (this);
    painter.eraseRect (rect ());
	painter.setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (max && DownSpeeds_.size ())
	{
		painter.setPen (DownColor_);
		PaintSingle (max, DownSpeeds_, &painter);
		painter.setPen (UpColor_);
		PaintSingle (max, UpSpeeds_, &painter);
	}
}

void GraphWidget::PaintSingle (quint64 max, const QList<quint64>& speeds,
		QPainter *painter)
{
	int prevX = width () - 1;
	int i = speeds.size () - 1;
	double prevY = height () * (1 - static_cast<double> (speeds.at (i)) / static_cast<double> (max));
	for ( ; i >= 0; --i)
	{
		int x = width () - speeds.size () + i;
		double y = height () * (1 - static_cast<double> (speeds.at (i)) / static_cast<double> (max));
		painter->drawLine (QPointF (prevX, prevY), QPointF (x, y));
		prevX = x;
		prevY = y;
	}
}

