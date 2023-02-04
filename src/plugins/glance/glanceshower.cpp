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
#include <optional>
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
		struct GridInfo
		{
			int Rows_;
			int Cols_;

			explicit GridInfo (int widgetCount)
			{
				const auto sqr = static_cast<int> (std::sqrt (widgetCount));
				Rows_ = sqr;
				Cols_ = sqr;
				if (Rows_ * Cols_ < widgetCount)
					++Cols_;
				if (Rows_ * Cols_ < widgetCount)
					++Rows_;
			}
		};

		auto GetScreenGeometry ()
		{
			const auto window = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
			return QApplication::desktop ()->screenGeometry (window);
		}

		QSizeF GetCellSize (const GridInfo& grid)
		{
			const auto& geometry = GetScreenGeometry ();
			return
			{
				static_cast<qreal> (geometry.width () / grid.Cols_),
				static_cast<qreal> (geometry.height () / grid.Rows_),
			};
		}

		constexpr auto Subscale = 2;

		auto GetTabSize (ICoreTabWidget& ictw)
		{
			return ictw.Widget (ictw.CurrentIndex ())->size () / Subscale;
		}

		auto GetRenderingScaleFactor (QSize tabSize, QSizeF cellSize)
		{
			const auto cellContentsSize = cellSize * 0.8;
			return std::min (cellContentsSize.width () / tabSize.width (),
					cellContentsSize.height () / tabSize.height ());
		}

		constexpr auto AnimationLength = 400;

		auto MakePosAnimator (GlanceItem& item, int row, int column, QSizeF cellSize)
		{
			auto anim = new QPropertyAnimation { &item, "Pos" };
			anim->setDuration (AnimationLength);
			anim->setStartValue (item.pos ());
			anim->setEndValue (QPointF { column * cellSize.width (), row * cellSize.height () });
			anim->setEasingCurve (QEasingCurve::OutSine);
			return anim;
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

		const GridInfo grid { count };
		const auto [rows, cols] = grid;

		const QSizeF cellSize = GetCellSize (grid);
		const auto tabSize = GetTabSize (TabWidget_);
		const auto scaleFactor = GetRenderingScaleFactor (tabSize, cellSize);

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

				QPixmap pixmap { tabSize * Subscale };
				w->render (&pixmap);
				pixmap = pixmap.scaled (tabSize,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

				//Close button
				const int buttonSize = 25;
				const int buttonLeft = tabSize.width () - buttonSize * 2;
				const int buttonTop = buttonSize;
				const QRect buttonRect { QPoint { buttonLeft, buttonTop }, QSize { buttonSize, buttonSize } };

				{
					QPainter p (&pixmap);
					QPen pen (Qt::black);
					pen.setWidth (2 / scaleFactor + 1);
					p.setPen (pen);
					p.drawRect (QRect (QPoint (0, 0), tabSize));
				}

				const auto item = new GlanceItem (pixmap, buttonRect);
				item->SetIndex (idx);
				connect (item,
						&GlanceItem::tabSelected,
						this,
						&GlanceShower::HandleSelected);
				connect (item,
						&GlanceItem::tabClosed,
						this,
						&GlanceShower::HandleClosed);

				Items_ << item;

				Scene_->addItem (item);
				item->setTransformOriginPoint (tabSize.width () / 2., tabSize.height () / 2.);
				item->setScale (scaleFactor);
				item->SetIdealScale (scaleFactor);
				item->setOpacity (0);

				const auto pair = new QParallelAnimationGroup;

				pair->addAnimation (MakePosAnimator (*item, row, column, cellSize));

				const auto opacityAnim = new QPropertyAnimation (item, "Opacity");
				opacityAnim->setDuration (AnimationLength);
				opacityAnim->setStartValue (0.);
				opacityAnim->setEndValue (1.);
				pair->addAnimation (opacityAnim);

				animGroup->addAnimation (pair);
			}

		setScene (Scene_);

		setGeometry (GetScreenGeometry ());
		animGroup->start ();

		show ();
	}

	namespace
	{
		void HandleNav (const QVector<GlanceItem*>& items,
				std::optional<int> currentItem, int delta, int wrapAround)
		{
			if (currentItem)
				items [*currentItem]->SetCurrent (false);

			if (currentItem)
				*currentItem += delta;

			if (!currentItem || currentItem < 0 || currentItem >= items.size ())
				currentItem = wrapAround;

			items [*currentItem]->SetCurrent (true);
		}

		std::optional<int> GetCurrentItem (const QVector<GlanceItem*>& items)
		{
			for (int i = 0; i < items.size (); ++i)
				if (items [i]->IsCurrent ())
					return i;

			return {};
		}
	}

	void GlanceShower::keyPressEvent (QKeyEvent *e)
	{
		if (e->key () == Qt::Key_Escape)
		{
			Finalize ();
			return;
		}

		const int count = TabWidget_.WidgetCount ();
		auto [rows, cols] = GridInfo { count };

		const auto& currentItem = GetCurrentItem (Items_);

		const auto next = [=, this] { HandleNav (Items_, currentItem, +1, 0); };
		const auto prev = [=, this] { HandleNav (Items_, currentItem, -1, count - 1); };

		switch (e->key ())
		{
		case Qt::Key_Right:
			next ();
			break;
		case Qt::Key_Left:
			prev ();
			break;
		case Qt::Key_Down:
			if (rows == 1)
				next ();
			else
			{
				const auto wrapAround = currentItem ? *currentItem % cols : 0;
				HandleNav (Items_, currentItem, +cols, wrapAround);
			}
			break;
		case Qt::Key_Up:
			if (rows == 1)
				prev ();
			else
			{
				auto wrapAround = currentItem ? *currentItem + (rows - 1) * cols : 0;
				if (wrapAround >= Items_.size ())
					wrapAround -= cols;
				HandleNav (Items_, currentItem, -cols, wrapAround);
			}
			break;
		case Qt::Key_Return:
			if (currentItem)
				HandleSelected (*currentItem);
			break;
		default:
			QGraphicsView::keyPressEvent (e);
			break;
		}
	}

	void GlanceShower::HandleClosed (int idx)
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

		const GridInfo grid { count };
		const auto [rows, cols] = grid;

		const QSizeF cellSize = GetCellSize (grid);
		const auto scaleFactor = GetRenderingScaleFactor (GetTabSize (TabWidget_), cellSize);

		const auto anim = new QParallelAnimationGroup;
		for (int row = 0; row < rows; ++row)
			for (int column = 0;
					column < cols && column + row * cols < count;
					++column)
			{
				const int idx = column + row * cols;
				const auto item = Items_ [idx];
				item->SetIndex (idx);
				item->SetIdealScale (scaleFactor);

				auto pair = new QParallelAnimationGroup;
				pair->addAnimation (MakePosAnimator (*item, row, column, cellSize));

				auto scaleAnim = new QPropertyAnimation (item, "Scale");
				scaleAnim->setDuration (AnimationLength);
				scaleAnim->setStartValue (item->scale ());
				scaleAnim->setEndValue (scaleFactor);
				pair->addAnimation (scaleAnim);

				anim->addAnimation (pair);
			}
		anim->start ();
	}

	void GlanceShower::HandleSelected (int idx)
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
