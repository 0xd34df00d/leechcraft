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
			return;

		QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup;

		int sqr = std::sqrt ((double)count);
		int rows = sqr;
		int cols = sqr;
		if (rows * cols < count)
			++cols;
		if (rows * cols < count)
			++rows;

		int width = Core::Instance ().GetReallyMainWindow ()->width ();
		int height = Core::Instance ().GetReallyMainWindow ()->height ();

		int singleW = width / cols;
		int singleH = height / rows;

		int wW = singleW * 4 / 5;
		int wH = singleH * 4 / 5;

		qreal scaleFactor = 0;
		QSize sSize;

		qreal fourth = std::sqrt (std::sqrt ((double)count));

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
				pixmap = pixmap.scaled (sSize);

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

		setGeometry (Core::Instance ().GetReallyMainWindow ()->geometry ());
		animGroup->start ();
		show ();
	}

	void GlanceShower::keyPressEvent (QKeyEvent *e)
	{
		if (e->key () == Qt::Key_Escape)
			deleteLater ();
		else
			QGraphicsView::keyPressEvent (e);
	}

	void GlanceShower::handleClicked (int idx)
	{
		TabWidget_->setCurrentIndex (idx);
		deleteLater ();
	}
};

