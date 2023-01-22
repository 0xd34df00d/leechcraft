/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glanceshower.h"
#include <cmath>
#include <limits>
#include <QLabel>
#include <QGraphicsPixmapItem>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QProgressDialog>
#include <QPropertyAnimation>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/ihavetabs.h>
#include "glanceitem.h"

namespace LC::Plugins::Glance
{
	GlanceShower::GlanceShower (QWidget *parent)
	: QGraphicsView (parent)
	, Scene_ (new QGraphicsScene)
	{
		setWindowFlags (Qt::Dialog |
				Qt::WindowStaysOnTopHint |
				Qt::FramelessWindowHint);
		setAttribute (Qt::WA_TranslucentBackground);
		setStyleSheet ("background: transparent"_qs);
		setOptimizationFlag (DontSavePainterState);
		Scene_->setItemIndexMethod (QGraphicsScene::NoIndex);
		setRenderHints (QPainter::Antialiasing | QPainter::TextAntialiasing);
	}

	void GlanceShower::SetTabWidget (ICoreTabWidget *tw)
	{
		TabWidget_ = tw;
	}

	namespace
	{
		auto GetWindowGeometry ()
		{
			const auto window = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
			return QApplication::desktop ()->screenGeometry (window);
		}
	}

	void GlanceShower::Start ()
	{
		if (!TabWidget_)
		{
			qWarning () << Q_FUNC_INFO
				<< "no tab widget set";
			return;
		}

		const int count = TabWidget_->WidgetCount ();
		if (count < 2)
		{
			emit finished ();
			return;
		}

		QAnimationGroup *animGroup = new QParallelAnimationGroup;

		const int sqr = std::sqrt (static_cast<double> (count));
		int rows = sqr;
		int cols = sqr;
		if (rows * cols < count)
			++cols;
		if (rows * cols < count)
			++rows;

		const auto& screenGeom = GetWindowGeometry ();
		const int width = screenGeom.width ();
		const int height = screenGeom.height ();

		const int singleW = width / cols;
		const int singleH = height / rows;

		const int wW = singleW * 4 / 5;
		const int wH = singleH * 4 / 5;

		qreal scaleFactor = 0;

		const int animLength = 400;

		QProgressDialog pg;
		pg.setMinimumDuration (1000);
		pg.setRange (0, count);

		for (int row = 0; row < rows; ++row)
			for (int column = 0;
					column < cols && column + row * cols < count;
					++column)
			{
				const int idx = column + row * cols;
				pg.setValue (idx);
				QWidget *w = TabWidget_->Widget (idx);

				if (!SSize_.isValid ())
					SSize_ = w->size () / 2;
				if (SSize_ != w->size ())
					w->resize (SSize_ * 2);

				if (std::fabs (scaleFactor) < std::numeric_limits<qreal>::epsilon ())
					scaleFactor = std::min (static_cast<qreal> (wW) / SSize_.width (),
							static_cast<qreal> (wH) / SSize_.height ());

				QPixmap pixmap (SSize_ * 2);
				w->render (&pixmap);
				pixmap = pixmap.scaled (SSize_,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

				//Close button
				const int buttonSize = 25;
				const int buttonLeft = SSize_.width() - buttonSize * 2;
				const int buttonTop = buttonSize;
				const QRect buttonRect { QPoint { buttonLeft, buttonTop }, QSize { buttonSize, buttonSize } };

				{
					QPainter p (&pixmap);
					QPen pen (Qt::black);
					pen.setWidth (2 / scaleFactor + 1);
					p.setPen (pen);
					p.drawRect (QRect (QPoint (0, 0), SSize_));

				}

				const auto item = new GlanceItem (pixmap, buttonRect);
				item->SetIndex (idx);
				connect (item,
						SIGNAL (clicked (int, bool)),
						this,
						SLOT (handleClicked (int, bool)));

				Scene_->addItem (item);
				item->setTransformOriginPoint (SSize_.width () / 2, SSize_.height () / 2);
				item->setScale (scaleFactor);
				item->SetIdealScale (scaleFactor);
				item->setOpacity (0);
				item->moveBy (column * singleW, row * singleH);

				QAnimationGroup *pair = new QParallelAnimationGroup;

				const auto posAnim = new QPropertyAnimation (item, "Pos");
				posAnim->setDuration (animLength);
				posAnim->setStartValue (QPointF (0, 0));
				posAnim->setEndValue (QPointF (column * singleW, row * singleH));
				posAnim->setEasingCurve (QEasingCurve::OutSine);
				pair->addAnimation (posAnim);

				const auto opacityAnim = new QPropertyAnimation (item, "Opacity");
				opacityAnim->setDuration (animLength);
				opacityAnim->setStartValue (0.);
				opacityAnim->setEndValue (1.);
				pair->addAnimation (opacityAnim);

				animGroup->addAnimation (pair);
			}

		setScene (Scene_);

		setGeometry (screenGeom);
		animGroup->start ();

		for (const auto item : items ())
			qgraphicsitem_cast<GlanceItem*> (item)->SetItemList (items ());

		show ();
	}

	void GlanceShower::keyPressEvent (QKeyEvent *e)
	{
		if (e->key () == Qt::Key_Escape)
			Finalize ();
		else
		{
			const auto& glanceItemList = Util::Map (items (),
					[] (auto item) { return qgraphicsitem_cast<GlanceItem*> (item); });

			int currentItem = -1;
			const int count = TabWidget_->WidgetCount ();

			const int sqrt = std::sqrt (static_cast<double> (count));
			int rows = sqrt;
			int cols = sqrt;
			if (rows * cols < count)
				++cols;
			if (rows * cols < count)
				++rows;

			for (int i = 0; i < count; ++i)
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
					const auto event = new QKeyEvent (QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
					QCoreApplication::postEvent (this, event);
				}
				else
					if (currentItem < 0)
						glanceItemList [0]->SetCurrent (true);
					else if (currentItem + cols < count)
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
					const auto event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
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

	void GlanceShower::handleClicked (int idx, bool close)
	{
		if (close)
		{
			qobject_cast<ITabWidget*> (TabWidget_->Widget (idx))->Remove ();
			Scene_->removeItem (Scene_->items () [idx]);
			if (Scene_->items ().size () < 2)
			{
				Finalize ();
				return;
			}
			//Now rearrange and resize all the rest items
			const int count = TabWidget_->WidgetCount ();
			const int sqr = std::sqrt (static_cast<double> (count));
			int rows = sqr;
			int cols = sqr;
			if (rows * cols < count)
				++cols;
			if (rows * cols < count)
				++rows;

			const auto& screenGeom = GetWindowGeometry ();
			const int width = screenGeom.width ();
			const int height = screenGeom.height ();

			const int singleW = width / cols;
			const int singleH = height / rows;

			const int wW = singleW * 4 / 5;
			const int wH = singleH * 4 / 5;

			const int animLength = 400;

			const auto anim = new QParallelAnimationGroup;

			const auto scaleFactor = std::min (static_cast<qreal> (wW) / SSize_.width (),
					static_cast<qreal> (wH) / SSize_.height ());

			const auto allItems = items ();
			for (int row = 0; row < rows; ++row)
				for (int column = 0;
						column < cols && column + row * cols < count;
						++column)
				{
					const int idx = column + row * cols;
					const auto item = qgraphicsitem_cast<GlanceItem*> (allItems [idx]);
					item->SetIndex (idx);
					item->SetIdealScale (scaleFactor);

					auto pair = new QParallelAnimationGroup ();

					auto posAnim = new QPropertyAnimation (item, "Pos");
					posAnim->setDuration (animLength);
					posAnim->setStartValue (item->pos ());
					posAnim->setEndValue (QPointF (column * singleW, row * singleH));
					posAnim->setEasingCurve (QEasingCurve::OutSine);
					pair->addAnimation (posAnim);

					auto scaleAnim = new QPropertyAnimation (item, "Scale");
					scaleAnim->setDuration (animLength);
					scaleAnim->setStartValue (item->scale ());
					scaleAnim->setEndValue (scaleFactor);
					pair->addAnimation (scaleAnim);

					anim->addAnimation (pair);
				}
			anim->start ();
		}
		else
		{
			TabWidget_->setCurrentTab (idx);
			Finalize ();
		}
	}

	void GlanceShower::Finalize ()
	{
		emit finished ();
		deleteLater ();
	}

	void GlanceShower::mousePressEvent (QMouseEvent *e)
	{
		QGraphicsView::mousePressEvent (e);
		e->accept ();
		if (!itemAt (e->pos ()))
			Finalize ();
	}
}
