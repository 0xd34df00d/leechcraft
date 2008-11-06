#include <QPaintEvent>
#include <QPainter>
#include <QPolygonF>
#include <QtDebug>
#include "graphwidget.h"

GraphWidget::GraphWidget (const QColor& color, QWidget *parent)
: QWidget (parent)
, Color_ (color)
{
    setAutoFillBackground (true);
    setBackgroundRole (QPalette::Window);
    setPalette (Qt::black);
}

void GraphWidget::PushSpeed (quint64 speed)
{
    if (Speeds_.size () == width ())
        Speeds_.removeAt (0);
    Speeds_.append (speed);
    update ();
}

void GraphWidget::paintEvent (QPaintEvent*)
{
    quint64 max = 0;
    for (int i = 0; i < Speeds_.size (); ++i)
        if (Speeds_.at (i) > max)
            max = Speeds_.at (i);

    QPainter painter (this);
    painter.setPen (Color_);
    painter.eraseRect (rect ());
    painter.setBrush (QBrush (Color_));
    if (max)
        for (int i = Speeds_.size () - 1; i >= 0; --i)
        {
            int x = width () - Speeds_.size () + i;
            double y = height () * (1 - static_cast<double> (Speeds_.at (i)) / static_cast<double> (max));
            painter.drawLine (QPointF (x, height ()), QPointF (x, y));
        }
}

