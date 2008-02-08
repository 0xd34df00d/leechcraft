#include <QtDebug>
#include <QPaintEvent>
#include <QPainter>
#include <cmath>
#include "gamewidget.h"
#include "particle.h"

GameWidget::GameWidget (QWidget *parent)
: QWidget (parent)
, FirstUpdate_ (true)
{
   update ();
}

void GameWidget::DoDelayedInit ()
{
}

GameWidget::~GameWidget ()
{
   delete Ship_;
}

void GameWidget::rotateLeft ()
{
   Ship_->RotateBy (-80);
   Ship_->Step ();
   update ();
}

void GameWidget::rotateRight ()
{
   Ship_->RotateBy (80);
   Ship_->Step ();
   update ();
}

void GameWidget::speedUp ()
{
   Ship_->SpeedUp (0.2);
   Ship_->Step ();
   update ();
}

void GameWidget::paintEvent (QPaintEvent *e)
{
   if (FirstUpdate_)
   {
      Ship_ = new Ship (width () / 2, height () / 2, 0);
      qsrand (32768);
      for (int i = 0; i < 50; ++i)
         Stars_.append (QPoint (qrand () % width (), qrand () % height ()));
      FirstUpdate_ = false;
   }

   QPainter painter;
   painter.begin (this);
   painter.setBrush (Qt::black);
   painter.drawRect (rect ());
   painter.setPen (Qt::white);
   for (int i = 0; i < Stars_.size (); ++i)
      painter.drawPoint (Stars_.at (i));
   QMatrix moveMatrix;
   QImage ship = Ship_->Draw ();
   moveMatrix.translate (ship.width () / 2, ship.height () / 2);
   painter.drawImage (QPoint (Ship_->GetX (), Ship_->GetY ()), ship.transformed (moveMatrix));
   painter.end ();
   e->accept ();
}

