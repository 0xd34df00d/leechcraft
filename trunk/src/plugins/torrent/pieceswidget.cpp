#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <QtDebug>
#include "pieceswidget.h"

PiecesWidget::PiecesWidget (QWidget *parent)
: QLabel (parent)
{
}

void PiecesWidget::setPieceMap (const std::vector<bool>& pieces)
{
   Pieces_ = pieces;

   update ();
}

QList<QPair<int, int> > FindTrues (const std::vector<bool>& pieces)
{
   QList<QPair<int, int> > result;
   bool prevVal = pieces [0];
   int prevPos = 0;
   for (int i = 1; i < pieces.size (); ++i)
      if (pieces [i] != prevVal)
      {
         if (prevVal)
            result << qMakePair (prevPos, i);
         prevPos = i;
         prevVal = 1 - prevVal;
      }

   if (!prevPos && prevVal)
      result << qMakePair<int, int> (0, pieces.size ());
   else if (prevVal && result.last ().second != pieces.size () - 1)
      result << qMakePair<int, int> (prevPos, pieces.size ());

   return result;
}

void PiecesWidget::paintEvent (QPaintEvent *e)
{
   int s = Pieces_.size ();
   QPainter painter (this);
   if (!s)
   {
      painter.setBackgroundMode (Qt::OpaqueMode);
      painter.setBackground (Qt::white);
      painter.end ();
      return;
   }

   int h = height ();

   painter.fillRect (0, 0, width (), height (), QBrush (Qt::red));

   qreal scaleFactor = static_cast<qreal> (width ()) / static_cast<qreal> (s);

   QList<QPair<int, int> > trues = FindTrues (Pieces_);
   for (int i = 0; i < trues.size (); ++i)
   {
      QPair<int, int> pair = trues.at (i);
      QPointF first (scaleFactor * pair.first, 0);
      QPointF second (scaleFactor * pair.second, h);
      painter.fillRect (QRectF (first, second), QBrush (Qt::darkGreen));
   }
   painter.end ();

   e->accept ();
}

