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
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/ihavetabs.h>
#include "glanceitem.h"

namespace LC::Plugins::Glance
{
	GlanceShower::GlanceShower (ICoreTabWidget& tw, QWidget *parent)
	: QGraphicsView { parent }
	, TabWidget_ { tw }
	, Scene_ { new QGraphicsScene }
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

	namespace
	{
		auto GetWindowGeometry ()
		{
			const auto window = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
			return QApplication::desktop ()->screenGeometry (window);
		}

		auto GetGridInfo (int widgetCount)
		{
			const auto sqr = static_cast<int> (std::sqrt (widgetCount));
			int rows = sqr;
			int cols = sqr;
			if (rows * cols < widgetCount)
				++cols;
			if (rows * cols < widgetCount)
				++rows;

			struct
			{
				int Rows_;
				int Cols_;
			} grid { rows, cols };
			return grid;
		}
	}

	void GlanceShower::Start ()
	{
		const int count = TabWidget_.WidgetCount ();
		if (count < 2)
		{
			Finalize ();
			return;
		}

		Items_.reserve (count);

		const auto animGroup = new QParallelAnimationGroup;

		const auto [rows, cols] = GetGridInfo (count);

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
				const auto w = TabWidget_.Widget (idx);

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
						&GlanceItem::tabSelected,
						this,
						&GlanceShower::handleSelected);
				connect (item,
						&GlanceItem::tabClosed,
						this,
						&GlanceShower::handleClosed);

				Items_ << item;

				Scene_->addItem (item);
				item->setTransformOriginPoint (SSize_.width () / 2, SSize_.height () / 2);
				item->setScale (scaleFactor);
				item->SetIdealScale (scaleFactor);
				item->setOpacity (0);
				item->moveBy (column * singleW, row * singleH);

				const auto pair = new QParallelAnimationGroup;

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

		show ();
	}

	namespace
	{
		void HandleHorizontalNav (const QVector<GlanceItem*>& items,
				int currentItem, int delta, int wrapAround)
		{
			if (currentItem != -1)
				items [currentItem]->SetCurrent (false);

			if (currentItem != -1)
				currentItem += delta;

			if (currentItem < 0 || currentItem >= items.size ())
				currentItem = wrapAround;

			items [currentItem]->SetCurrent (true);
		}
	}

	void GlanceShower::keyPressEvent (QKeyEvent *e)
	{
		if (e->key () == Qt::Key_Escape)
			Finalize ();
		else
		{
			int currentItem = -1;
			const int count = TabWidget_.WidgetCount ();

			auto [rows, cols] = GetGridInfo (count);

			for (int i = 0; i < count; ++i)
				if (Items_ [i]->IsCurrent ())
					currentItem = i;

			switch (e->key ())
			{
			case Qt::Key_Right:
				HandleHorizontalNav (Items_, currentItem, +1, 0);
				break;
			case Qt::Key_Left:
				HandleHorizontalNav (Items_, currentItem, -1, count - 1);
				break;
			case Qt::Key_Down:
				if (count < 3)
				{
					const auto event = new QKeyEvent (QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
					QCoreApplication::postEvent (this, event);
				}
				else
					if (currentItem < 0)
						Items_ [0]->SetCurrent (true);
					else if (currentItem + cols < count)
					{
						Items_ [currentItem]->SetCurrent (false);
						Items_ [currentItem + cols]->SetCurrent (true);
					}
					else
					{
						Items_ [currentItem]->SetCurrent (false);
						while ((currentItem - cols * (rows - 1)) <  0)
							rows--;
						Items_ [currentItem - cols * (rows - 1)]->SetCurrent (true);
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
						Items_ [0]->SetCurrent (true);
					else
						if (currentItem >= cols)
						{
							Items_ [currentItem]->SetCurrent (false);
							Items_ [currentItem - cols]->SetCurrent (true);
						}
						else
						{
							Items_ [currentItem]->SetCurrent (false);
							while ((currentItem + cols * (rows - 1)) > count - 1)
								rows--;
							Items_ [currentItem + cols * (rows - 1)]->SetCurrent (true);
						}
				break;
			case Qt::Key_Return:
				if (currentItem >= 0)
					handleSelected (currentItem);
				break;
			default:
				QGraphicsView::keyPressEvent (e);
			}
		}
	}

	void GlanceShower::handleClosed (int idx)
	{
		qobject_cast<ITabWidget*> (TabWidget_.Widget (idx))->Remove ();

		const auto removedItem = Items_.takeAt (idx);
		Scene_->removeItem (removedItem);
		if (Items_.size () < 2)
		{
			Finalize ();
			return;
		}

		//Now rearrange and resize all the rest items
		const int count = TabWidget_.WidgetCount ();
		const auto [rows, cols] = GetGridInfo (count);

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

	void GlanceShower::handleSelected (int idx)
	{
		TabWidget_.setCurrentTab (idx);
		Finalize ();
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
