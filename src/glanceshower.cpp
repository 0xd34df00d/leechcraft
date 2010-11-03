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

#include "glanceshower.h"
#include <cmath>
#include <QTabWidget>
#include <QLabel>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QProgressDialog>
#include <QPropertyAnimation>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QtDebug>
#include "core.h"
#include "mainwindow.h"
#include "glanceitem.h"

namespace LeechCraft
{
	GlanceShower::GlanceShower (QWidget *parent)
	: QGraphicsView (parent)
	, TabWidget_ (0)
	, Scene_ (new QGraphicsScene)
	, Shown_ (false)
	{
		setWindowFlags (Qt::WindowStaysOnTopHint |
				Qt::FramelessWindowHint);
		setAttribute (Qt::WA_TranslucentBackground);
		setStyleSheet ("background: transparent");
		setOptimizationFlag (DontSavePainterState);
		Scene_->setItemIndexMethod (QGraphicsScene::NoIndex);
		setRenderHints (QPainter::HighQualityAntialiasing);
	}

	void GlanceShower::SetTabWidget (QTabWidget *tw)
	{
		TabWidget_ = tw;
	}

	void GlanceShower::Start ()
	{
		if (!TabWidget_)
		{
			qWarning () << Q_FUNC_INFO
				<< "no tab widget set";
			return;
		}

		int count = TabWidget_->count ();
		if (count < 2)
		{
			emit finished (true);
			return;
		}

		QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup;

		int sqr = std::sqrt ((double)count);
		int rows = sqr;
		int cols = sqr;
		if (rows * cols < count)
			++cols;
		if (rows * cols < count)
			++rows;

		QRect screenGeom = QApplication::desktop ()->
				screenGeometry (Core::Instance ().GetReallyMainWindow ());
		int width = screenGeom.width ();
		int height = screenGeom.height ();

		int singleW = width / cols;
		int singleH = height / rows;

		int wW = singleW * 4 / 5;
		int wH = singleH * 4 / 5;

		qreal scaleFactor = 0;
		QSize sSize;

		int animLength = 500 / (sqr);

		QProgressDialog pg;
		pg.setMinimumDuration (1000);
		pg.setRange (0, count);

		for (int row = 0; row < rows; ++row)
			for (int column = 0;
					column < cols && column + row * cols < count;
					++column)
			{
				int idx = column + row * cols;
				pg.setValue (idx);
				QWidget *w = TabWidget_->widget (idx);

				if (!sSize.isValid ())
					sSize = w->size () / 2;
				if (sSize != w->size ())
					w->resize (sSize * 2);

				if (!scaleFactor)
					scaleFactor = std::min (static_cast<qreal> (wW) / sSize.width (),
							static_cast<qreal> (wH) / sSize.height ());

				QPixmap pixmap (sSize * 2);
				w->render (&pixmap);
				pixmap = pixmap.scaled (sSize,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

				{
					QPainter p (&pixmap);
					QPen pen (Qt::black);
					pen.setWidth (2 / scaleFactor + 1);
					p.setPen (pen);
					p.drawRect (QRect (QPoint (0, 0), sSize));
				}

				GlanceItem *item = new GlanceItem (pixmap);
				item->SetIndex (idx);
				connect (item,
						SIGNAL (clicked (int)),
						this,
						SLOT (handleClicked (int)));

				Scene_->addItem (item);
				item->setTransformOriginPoint (sSize.width () / 2, sSize.height () / 2);
				item->setScale (scaleFactor);
				item->SetIdealScale (scaleFactor);
				item->setOpacity (0);
				item->moveBy (column * singleW, row * singleH);

				QParallelAnimationGroup *pair = new QParallelAnimationGroup;

				QPropertyAnimation *posAnim = new QPropertyAnimation (item, "Pos");
				posAnim->setDuration (animLength);
				posAnim->setStartValue (QPointF (0, 0));
				posAnim->setEndValue (QPointF (column * singleW, row * singleH));
				posAnim->setEasingCurve (QEasingCurve::OutSine);
				pair->addAnimation (posAnim);

				QPropertyAnimation *opacityAnim = new QPropertyAnimation (item, "Opacity");
				opacityAnim->setDuration (animLength);
				opacityAnim->setStartValue (0.);
				opacityAnim->setEndValue (1.);
				pair->addAnimation (opacityAnim);

				animGroup->addAnimation (pair);
			}

		setScene (Scene_);

		setGeometry (screenGeom);
		animGroup->start ();

		Q_FOREACH (QGraphicsItem* item, items ())
		{
			GlanceItem *itm = qgraphicsitem_cast<GlanceItem*> (item);
			itm->SetItemList (items ());
		}

		show ();
	}

	void GlanceShower::keyPressEvent (QKeyEvent *e)
	{
		if (e->key () == Qt::Key_Escape)
			Finalize ();
		else
		{
			QList<GlanceItem*> glanceItemList;
			Q_FOREACH (QGraphicsItem* item, items ())
				glanceItemList << qgraphicsitem_cast<GlanceItem*> (item);

			int currentItem = -1;
			int count = TabWidget_->count ();

			int sqrt = std::sqrt ((double)count);
			int rows = sqrt;
			int cols = sqrt;
			if (rows * cols < count)
				++cols;
			if (rows * cols < count)
				++rows;

			for (int i = 0; i < count; i++)
				if (glanceItemList [i]->IsCurrent ())
					currentItem = i;


			switch (e->key ())
			{
			case Qt::Key_Right:
				if (currentItem < 0)
					glanceItemList [0]->SetCurrent (true);
				else
					if (currentItem < (count - 1))
					{
						glanceItemList [currentItem]->SetCurrent (false);
						glanceItemList [currentItem + 1]->SetCurrent (true);
					}
					else
					{
						glanceItemList [currentItem]->SetCurrent (false);
						glanceItemList [0]->SetCurrent (true);
					}
				break;
			case Qt::Key_Left:
				if (currentItem < 0)
					glanceItemList [count - 1]->SetCurrent (true);
				else
					if (currentItem > 0)
					{
						glanceItemList [currentItem]->SetCurrent (false);
						glanceItemList [currentItem - 1]->SetCurrent (true);
					}
					else
					{
						glanceItemList [currentItem]->SetCurrent (false);
						glanceItemList [count - 1]->SetCurrent (true);
					}
				break;
			case Qt::Key_Down:
				if (count < 3)
				{
					QKeyEvent *event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
					QCoreApplication::postEvent (this, event);
				}
				else
					if (currentItem < 0)
						glanceItemList [0]->SetCurrent (true);
					else
						if (currentItem + cols < count)
						{
							glanceItemList [currentItem]->SetCurrent (false);
							glanceItemList [currentItem + cols]->SetCurrent (true);
						}
						else
						{
							glanceItemList [currentItem]->SetCurrent (false);
							while ((currentItem - cols * (rows - 1)) <  0)
								rows--;
							glanceItemList [currentItem - cols * (rows - 1)]->SetCurrent (true);
						}
				break;
			case Qt::Key_Up:
				if (count < 3)
				{
					QKeyEvent *event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
					QCoreApplication::postEvent (this, event);
				}
				else				
					if (currentItem < 0)
						glanceItemList [0]->SetCurrent (true);
					else
						if (currentItem >= cols)
						{
							glanceItemList [currentItem]->SetCurrent (false);
							glanceItemList [currentItem - cols]->SetCurrent (true);
						}
						else
						{
							glanceItemList [currentItem]->SetCurrent (false);
							while ((currentItem + cols * (rows - 1)) > count - 1)
								rows--;
							glanceItemList [currentItem + cols * (rows - 1)]->SetCurrent (true);
						}
				break;
			case Qt::Key_Return:
				if (currentItem >= 0)
					handleClicked (currentItem);
				break;
			default:
				QGraphicsView::keyPressEvent (e);
			}
		}
	}

	void GlanceShower::handleClicked (int idx)
	{		
		TabWidget_->setCurrentIndex (idx);
		Finalize ();
	}

	void GlanceShower::Finalize ()
	{
		emit finished (true);
		deleteLater ();
	}

	void GlanceShower::mousePressEvent (QMouseEvent *e)
	{
		QGraphicsView::mousePressEvent (e);
		e->accept ();
		if (!this->itemAt (e->pos ()))
			Finalize ();
	}
};

