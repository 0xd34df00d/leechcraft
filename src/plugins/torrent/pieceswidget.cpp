#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <QtDebug>
#include "pieceswidget.h"

PiecesWidget::PiecesWidget (QWidget *parent)
: QWidget (parent)
{
}

void PiecesWidget::setPieceMap (const std::vector<bool>& pieces)
{
	Pieces_ = pieces;

	update ();
}

void PiecesWidget::paintEvent (QPaintEvent *e)
{
	int s = Pieces_.size ();
	if (!s)
		return;

	int h = height ();

	QPainter painter (this);
	painter.setBackgroundMode (Qt::OpaqueMode);
	painter.setBackground (QBrush (Qt::white));

	qreal scaleFactor = s / width ();
	qDebug () << scaleFactor;
	painter.scale (scaleFactor, 1);

	for (int i = 0; i < s; ++i)
	{
		painter.setPen (QPen (QColor (Pieces_ [i] ? Qt::green : Qt::red)));
		painter.drawLine (i, 0, i, h);
	}
	painter.end ();

	e->accept ();
}

