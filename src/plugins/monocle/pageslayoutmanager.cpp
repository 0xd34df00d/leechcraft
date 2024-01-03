/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pageslayoutmanager.h"
#include <QGraphicsScene>
#include <QScrollBar>
#include <QTimer>
#include <QtDebug>
#include <util/sll/unreachable.h>
#include "interfaces/monocle/idynamicdocument.h"
#include "pagesview.h"
#include "pagegraphicsitem.h"
#include "smoothscroller.h"
#include "common.h"

namespace LC
{
namespace Monocle
{
	const int Margin = 10;

	PagesLayoutManager::PagesLayoutManager (PagesView *view, SmoothScroller *scroller, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scroller_ (scroller)
	, Scene_ (view->scene ())
	, LayMode_ (LayoutMode::OnePage)
	, ScaleMode_ (ScaleMode::FitWidth)
	{
		connect (View_,
				SIGNAL (sizeChanged ()),
				this,
				SLOT (scheduleRelayout ()),
				Qt::QueuedConnection);
	}

	void PagesLayoutManager::HandleDoc (IDocument_ptr doc, const QList<PageGraphicsItem*>& pages)
	{
		CurrentDoc_ = doc;
		Pages_ = pages;
		Rotation_ = 0;
		emit rotationUpdated (0);

		PageRotations_ = QVector<double> (pages.size (), 0);

		for (auto page : pages)
			page->SetLayoutManager (this);

		if (CurrentDoc_ && qobject_cast<IDynamicDocument*> (CurrentDoc_->GetQObject ()))
			connect (CurrentDoc_->GetQObject (),
					SIGNAL (pageSizeChanged (int)),
					this,
					SLOT (handlePageSizeChanged (int)));
	}

	const QList<PageGraphicsItem*>& PagesLayoutManager::GetPages () const
	{
		return Pages_;
	}

	LayoutMode PagesLayoutManager::GetLayoutMode () const
	{
		return LayMode_;
	}

	void PagesLayoutManager::SetLayoutMode (LayoutMode layMode)
	{
		if (layMode == LayMode_)
			return;

		LayMode_ = layMode;
		emit layoutModeChanged ();
	}

	int PagesLayoutManager::GetLayoutModeCount () const
	{
		return LayMode_ == LayoutMode::OnePage ? 1 : 2;
	}

	QPoint PagesLayoutManager::GetViewportCenter () const
	{
		const auto& rect = View_->viewport ()->contentsRect ();
		return QPoint (rect.width (), rect.height ()) / 2;
	}

	int PagesLayoutManager::GetCurrentPage () const
	{
		const auto& center = GetViewportCenter ();
		auto item = View_->itemAt (center - QPoint (1, 1));
		if (!item)
			item = View_->itemAt (center - QPoint (10, 10));
		auto pos = std::find_if (Pages_.begin (), Pages_.end (), [item] (const auto& e) { return e == item; });
		return pos == Pages_.end () ? -1 : std::distance (Pages_.begin (), pos);
	}

	void PagesLayoutManager::SetCurrentPage (int idx, bool immediate)
	{
		if (idx < 0 || idx >= Pages_.size ())
			return;

		const auto page = Pages_.at (idx);
		auto center = page->boundingRect ().bottomRight ();
		center.ry () = std::min (center.y (), static_cast<qreal> (View_->viewport ()->contentsRect ().height ()));
		center /= 2;

		const auto sceneCenter = page->mapToScene (center);
		if (immediate)
			View_->centerOn (sceneCenter);
		else
			Scroller_->SmoothCenterOn (sceneCenter.x (), sceneCenter.y ());
	}

	void PagesLayoutManager::SetScaleMode (ScaleMode mode)
	{
		ScaleMode_ = mode;
	}

	ScaleMode PagesLayoutManager::GetScaleMode () const
	{
		return ScaleMode_;
	}

	void PagesLayoutManager::SetFixedScale (double scale)
	{
		FixedScale_ = scale;
	}

	double PagesLayoutManager::GetCurrentScale () const
	{
		if (!CurrentDoc_)
			return 1;

		auto calcRatio = [this] (std::function<double (const QSize&)> dimGetter) -> double
		{
			if (Pages_.isEmpty ())
				return 1.0;
			int pageIdx = GetCurrentPage ();
			if (pageIdx < 0)
				pageIdx = 0;

			double dim = dimGetter (GetRotatedSize (pageIdx).toSize () + QSize (2 * HorMargin_, 2 * VertMargin_));
			auto size = View_->maximumViewportSize ();
			size.rwidth () -= View_->verticalScrollBar ()->size ().width ();
			size.rheight () -= View_->horizontalScrollBar ()->size ().height ();

			const int margin = 3;
			size.rwidth () -= 2 * margin;
			size.rheight () -= 2 * margin;

			const auto res = dimGetter (size) / dim;
			return res > 0 ? res : 1;
		};

		switch (ScaleMode_)
		{
		case ScaleMode::FitWidth:
		{
			auto ratio = calcRatio ([] (const QSize& size) { return size.width (); });
			if (LayMode_ != LayoutMode::OnePage)
				ratio /= 2;
			return ratio;
		}
		case ScaleMode::FitPage:
		{
			auto wRatio = calcRatio ([] (const QSize& size) { return size.width (); });
			if (LayMode_ != LayoutMode::OnePage)
				wRatio /= 2;
			auto hRatio = calcRatio ([] (const QSize& size) { return size.height (); });
			return std::min (wRatio, hRatio);
		}
		case ScaleMode::Fixed:
			return FixedScale_;
		}

		Util::Unreachable ();
	}

	void PagesLayoutManager::SetRotation (double angle)
	{
		Rotation_ = angle;
		Relayout ();
		emit rotationUpdated (angle);
	}

	void PagesLayoutManager::AddRotation (double dAngle)
	{
		SetRotation (GetRotation () + dAngle);
	}

	double PagesLayoutManager::GetRotation () const
	{
		return Rotation_;
	}

	void PagesLayoutManager::SetRotation (double angle, int page)
	{
		PageRotations_ [page] = angle;
		Relayout ();
		emit rotationUpdated (angle, page);
	}

	void PagesLayoutManager::AddRotation (double dAngle, int page)
	{
		SetRotation (dAngle + GetRotation (page), page);
	}

	double PagesLayoutManager::GetRotation (int page) const
	{
		return PageRotations_ [page];
	}

	void PagesLayoutManager::SetMargins (double horizontal, double vertical)
	{
		HorMargin_ = horizontal;
		VertMargin_ = vertical;
	}

	void PagesLayoutManager::Relayout ()
	{
		const auto scale = GetCurrentScale ();
		const auto pageWas = GetCurrentPage ();
		const auto pageObj = Pages_.value (pageWas);

		QPointF oldPageCenter;
		if (pageWas >= 0)
		{
			const auto& pagePos = pageObj->mapFromScene (View_->mapToScene (GetViewportCenter ()));
			const auto& bounding = pageObj->boundingRect ();
			if (bounding.width () && bounding.height ())
				oldPageCenter = QPointF { pagePos.x () / bounding.width (),
						pagePos.y () / bounding.height () };
		}

		for (auto item : Pages_)
		{
			const auto pageRotation = PageRotations_ [item->GetPageNum ()] + Rotation_;
			const auto& bounding = item->boundingRect ();
			item->setTransformOriginPoint (bounding.width () / 2, bounding.height () / 2);

			item->setRotation (pageRotation);

			item->SetScale (scale, scale);
		}

		qreal currentY = 0;
		qreal lastWidth = 0;
		qreal lastHeight = 0;
		for (int i = 0, pagesCount = Pages_.size (); i < pagesCount; ++i)
		{
			auto page = Pages_ [i];

			const auto& size = GetRotatedSize (i) * scale;
			const auto& srcSize = CurrentDoc_->GetPageSize (i) * scale;
			const auto yDiff = (size.height () - srcSize.height ()) / 2;

			switch (LayMode_)
			{
			case LayoutMode::OnePage:
				page->setPos (0, currentY + yDiff);
				currentY += size.height () + Margin;
				break;
			case LayoutMode::TwoPages:
			case LayoutMode::TwoPagesShifted:
			{
				bool isLeftPage = LayMode_ == LayoutMode::TwoPages ?
						i % 2 :
						!(i % 2);
				if (isLeftPage)
				{
					page->setPos (lastWidth + Margin / 3, currentY + yDiff);
					currentY += std::max (lastHeight, size.height ()) + Margin;
				}
				else
				{
					page->setPos (0, currentY + yDiff);
					lastWidth = size.width ();
					lastHeight = size.height ();
				}
				break;
			}
			}
		}

		Scene_->setSceneRect (Scene_->itemsBoundingRect ()
					.adjusted (-HorMargin_, -VertMargin_, 0, 0));

		SetCurrentPage (std::max (pageWas, 0), true);
		if (pageWas >= 0)
		{
			const auto& bounding = pageObj->boundingRect ();
			const QPointF newCenter { bounding.width () * oldPageCenter.x (),
					bounding.height () * oldPageCenter.y () };
			View_->centerOn (pageObj->mapToScene (newCenter));
		}

		if (RelayoutScheduled_)
		{
			RelayoutScheduled_ = false;
			emit scheduledRelayoutFinished ();
		}
	}

	QSizeF PagesLayoutManager::GetRotatedSize (int page) const
	{
		const auto& origSize = CurrentDoc_->GetPageSize (page);
		const auto rotation = Pages_.at (page)->rotation ();
		QTransform tf;
		tf.rotate (rotation);
		return tf.mapRect (QRectF { { 0, 0 }, origSize }).size ();
	}

	void PagesLayoutManager::scheduleSetRotation (double angle)
	{
		SetRotation (angle);
		emit rotationUpdated (angle);
	}

	void PagesLayoutManager::scheduleRelayout ()
	{
		if (RelayoutScheduled_)
			return;

		QTimer::singleShot (500,
				this,
				SLOT (handleRelayout ()));
		RelayoutScheduled_ = true;
	}

	void PagesLayoutManager::handleRelayout ()
	{
		if (!RelayoutScheduled_)
			return;

		Relayout ();
		emit scheduledRelayoutFinished ();
	}

	void PagesLayoutManager::handlePageSizeChanged (int)
	{
		scheduleRelayout ();
	}
}
}
