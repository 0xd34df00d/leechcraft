/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pageslayoutmanager.h"
#include <cmath>
#include <QGraphicsScene>
#include <QStyle>
#include <QTimer>
#include <QtDebug>
#include <util/sll/visitor.h>
#include <util/sll/unreachable.h>
#include <util/monocle/documentsignals.h>
#include "pagesview.h"
#include "pagegraphicsitem.h"
#include "smoothscroller.h"
#include "common.h"

namespace LC::Monocle
{
	const int Margin = 12;

	PagesLayoutManager::PagesLayoutManager (PagesView *view, SmoothScroller *scroller, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scroller_ (scroller)
	, Scene_ (view->scene ())
	{
		connect (View_,
				&PagesView::sizeChanged,
				this,
				&PagesLayoutManager::ScheduleRelayout,
				Qt::QueuedConnection);
	}

	void PagesLayoutManager::HandleDoc (IDocument_ptr doc, const QVector<PageGraphicsItem*>& pages)
	{
		CurrentDoc_ = std::move (doc);
		Pages_ = pages;
		Rotation_ = 0;
		PageRotations_ = QVector<double> (pages.size (), 0);
		emit rotationUpdated (0);

		for (auto page : pages)
			page->SetLayoutManager (this);

		if (const auto docSignals = CurrentDoc_ ? CurrentDoc_->GetDocumentSignals () : nullptr)
			connect (docSignals,
					&DocumentSignals::pageSizeChanged,
					this,
					&PagesLayoutManager::ScheduleRelayout);
	}

	const QVector<PageGraphicsItem*>& PagesLayoutManager::GetPages () const
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
		const auto& rect = View_->viewport ()->contentsRect ();
		return PosTracker_.GetNearbyPage (View_->mapToScene (QPoint { 0, rect.height () / 2 }));
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

	constexpr auto HorizTwoPageMargin = Margin / 3;

	double PagesLayoutManager::GetCurrentScale () const
	{
		if (!CurrentDoc_)
			return 1;

		auto adjustForLayout = [this] (QSizeF size)
		{
			switch (LayMode_)
			{
			case LayoutMode::OnePage:
				return size;
			case LayoutMode::TwoPages:
			case LayoutMode::TwoPagesShifted:
				return QSizeF
				{
					size.width () * 2 + HorizTwoPageMargin,
					size.height (),
				};
			}
		};

		auto calcRatio = [&] (auto dimGetter)
		{
			if (Pages_.isEmpty ())
				return 1.0;

			const int pageIdx = std::max (GetCurrentPage (), 0);
			const auto pageDim = dimGetter (adjustForLayout (GetRotatedSize (pageIdx)) + 2 * Margins_);
			auto viewSize = View_->maximumViewportSize ();
			viewSize.rwidth () -= View_->style ()->pixelMetric (QStyle::PM_ScrollBarExtent);

			const auto res = dimGetter (viewSize) / pageDim;
			return res > 0 ? res : 1;
		};

		return Util::Visit (ScaleMode_,
				[&] (FitWidth)
				{
					return calcRatio ([] (QSizeF size) { return size.width (); });
				},
				[&] (FitPage)
				{
					const auto wRatio = calcRatio ([] (QSizeF size) { return size.width (); });
					const auto hRatio = calcRatio ([] (QSizeF size) { return size.height (); });
					return std::min (wRatio, hRatio);
				},
				[] (FixedScale fixed) { return fixed.Scale_; });
	}

	namespace
	{
		double LimitAngle (double angle)
		{
			constexpr double MinAngle = -180;
			constexpr double MaxAngle = 180;
			if (angle >= MinAngle && angle <= MaxAngle)
				return angle;

			angle -= MinAngle;
			angle = std::remainder (angle, MaxAngle - MinAngle);
			angle += MinAngle;
			return angle;
		}

		double ComputeRotationChange (double curValue, double arg, RotationChange change)
		{
			switch (change)
			{
			case RotationChange::Set:
				return arg;
			case RotationChange::Add:
				return curValue + arg;
			}
			Util::Unreachable ();
		}
	}

	void PagesLayoutManager::SetRotation (double value, RotationChange change)
	{
		Rotation_ = LimitAngle (ComputeRotationChange (Rotation_, value, change));
		Relayout ();
		emit rotationUpdated (Rotation_);
	}

	double PagesLayoutManager::GetRotation () const
	{
		return Rotation_;
	}

	void PagesLayoutManager::SetPageRotation (int page, double value, RotationChange change)
	{
		auto& rotation = PageRotations_ [page];
		rotation = LimitAngle (ComputeRotationChange (rotation, value, change));
		Relayout ();
		emit rotationUpdated (rotation, page);
	}

	double PagesLayoutManager::GetPageRotation (int page) const
	{
		return PageRotations_ [page];
	}

	void PagesLayoutManager::SetMargins (QSizeF margins)
	{
		Margins_ = margins;
	}

	std::pair<QPointF, QSizeF> PagesLayoutManager::GetPagePos (int pageIdx, double scale) const
	{
		const auto& size = GetRotatedSize (pageIdx) * scale;
		const auto& srcSize = CurrentDoc_->GetPageSize (pageIdx) * scale;
		const auto yDiff = (size.height () - srcSize.height ()) / 2;
		return { { 0, yDiff }, size };
	}

	void PagesLayoutManager::LayoutOneCol (double scale) const
	{
		qreal currentY = 0;
		for (int i = 0, pagesCount = Pages_.size (); i < pagesCount; ++i)
		{
			const auto page = Pages_ [i];
			const auto [pos, size] = GetPagePos (i, scale);
			page->setPos (pos + QPointF { 0, currentY });
			currentY += size.height () + Margin;
		}
	}

	void PagesLayoutManager::LayoutTwoCols (double scale, bool firstSeparate) const
	{
		if (Pages_.isEmpty ())
			return;

		qreal currentY = 0;
		if (firstSeparate)
		{
			const auto [pos, size] = GetPagePos (0, scale);
			Pages_ [0]->setPos (pos + QPointF { (size.width () + HorizTwoPageMargin) / 2, currentY });
			currentY += size.height () + Margin;
		}

		for (int i = firstSeparate ? 1 : 0, pagesCount = Pages_.size (); i < pagesCount; i += 2)
		{
			const auto [leftPos, leftSize] = GetPagePos (i, scale);
			Pages_ [i]->setPos (leftPos + QPointF { 0, currentY });

			if (i == pagesCount - 1)
				break;

			const auto [rightPos, rightSize] = GetPagePos (i + 1, scale);
			Pages_ [i + 1]->setPos (rightPos + QPointF { leftSize.width () + HorizTwoPageMargin, currentY });

			currentY += std::max (leftSize.height (), rightSize.height ()) + Margin;
		}
	}

	void PagesLayoutManager::ApplyPagesGeometry (double scale)
	{
		for (auto item : Pages_)
		{
			const auto pageRotation = PageRotations_ [item->GetPageNum ()] + Rotation_;
			const auto& bounding = item->boundingRect ();
			item->setTransformOriginPoint (bounding.width () / 2, bounding.height () / 2);
			item->setRotation (pageRotation);
			item->SetScale (scale, scale);
		}
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
			if (bounding.isValid ())
				oldPageCenter = QPointF { pagePos.x () / bounding.width (),
						pagePos.y () / bounding.height () };
		}

		ApplyPagesGeometry (scale);

		switch (LayMode_)
		{
		case LayoutMode::OnePage:
			LayoutOneCol (scale);
			break;
		case LayoutMode::TwoPages:
			LayoutTwoCols (scale, false);
			break;
		case LayoutMode::TwoPagesShifted:
			LayoutTwoCols (scale, true);
			break;
		}

		PosTracker_.Update ();

		Scene_->setSceneRect (Scene_->itemsBoundingRect ().adjusted (-Margins_.width (), -Margins_.height (), 0, 0));

		SetCurrentPage (std::max (pageWas, 0), true);
		if (pageWas >= 0)
		{
			const auto& bounding = pageObj->boundingRect ();
			const QPointF newCenter { bounding.width () * oldPageCenter.x (),
					bounding.height () * oldPageCenter.y () };
			View_->centerOn (pageObj->mapToScene (newCenter));
		}

		if (RelayoutScheduled_)
			emit scheduledRelayoutFinished ();
		RelayoutScheduled_ = false;
	}

	QSizeF PagesLayoutManager::GetRotatedSize (int page) const
	{
		const auto& origSize = CurrentDoc_->GetPageSize (page);
		const auto rotation = Pages_.at (page)->rotation ();
		QTransform tf;
		tf.rotate (rotation);
		return tf.mapRect (QRectF { { 0, 0 }, origSize }).size ();
	}

	void PagesLayoutManager::ScheduleRelayout ()
	{
		if (RelayoutScheduled_)
			return;

		QTimer::singleShot (500,
				this,
				[this]
				{
					Relayout ();
					emit scheduledRelayoutFinished ();
				});
		RelayoutScheduled_ = true;
	}
}
