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
#include "common.h"

namespace LC::Monocle
{
	const int Margin = 12;

	PagesLayoutManager::PagesLayoutManager (PagesView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scene_ (view->scene ())
	{
		connect (View_,
				&PagesView::sizeChanged,
				this,
				&PagesLayoutManager::ScheduleRelayout,
				Qt::QueuedConnection);
	}

	void PagesLayoutManager::HandleDoc (IDocument *doc, const QVector<PageGraphicsItem*>& pages)
	{
		CurrentDoc_ = doc;
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

	int PagesLayoutManager::GetCurrentPage () const
	{
		return PosTracker_.GetNearbyPage (View_->GetCurrentCenter ().ClearedX ());
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

			const auto& margins = View_->contentsMargins ();
			const auto scrollBarWidth = View_->style ()->pixelMetric (QStyle::PM_ScrollBarExtent);
			// Margins might already include the scrollbar width on the second and subsequent relayouts,
			// so we need to ignore that to avoid making margins too big and ugly.
			// Hence, we take the minimum of the left/right margins,
			// since at least one of them won't include the scrollbar.
			viewSize.rwidth () -= scrollBarWidth + std::min (margins.left (), margins.right ()) * 2;

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

	std::pair<SceneAbsolutePos, QSizeF> PagesLayoutManager::GetPagePos (int pageIdx, double scale) const
	{
		const auto& size = GetRotatedSize (pageIdx) * scale;
		const auto& srcSize = CurrentDoc_->GetPageSize (pageIdx) * scale;
		const auto yDiff = (size.height () - srcSize.height ()) / 2;
		return { { QPointF { 0, yDiff } }, size };
	}

	void PagesLayoutManager::LayoutOneCol (double scale) const
	{
		qreal currentY = 0;
		for (int i = 0, pagesCount = Pages_.size (); i < pagesCount; ++i)
		{
			const auto page = Pages_ [i];
			const auto [pos, size] = GetPagePos (i, scale);
			page->setPos (pos.Shifted (0, currentY));
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
			Pages_ [0]->setPos (pos.Shifted ((size.width () + HorizTwoPageMargin) / 2, currentY));
			currentY += size.height () + Margin;
		}

		for (int i = firstSeparate ? 1 : 0, pagesCount = Pages_.size (); i < pagesCount; i += 2)
		{
			const auto [leftPos, leftSize] = GetPagePos (i, scale);
			Pages_ [i]->setPos (leftPos.Shifted (0, currentY));

			if (i == pagesCount - 1)
				break;

			const auto [rightPos, rightSize] = GetPagePos (i + 1, scale);
			Pages_ [i + 1]->setPos (rightPos.Shifted (leftSize.width () + HorizTwoPageMargin, currentY));

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
		const auto pageObj = Pages_.value (GetCurrentPage ());

		PageRelativePos oldPageCenter;
		if (pageObj)
			oldPageCenter = View_->GetCurrentCenter ().ToPageRelative (*pageObj);

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

		if (pageObj)
			View_->CenterOn (oldPageCenter.ToSceneAbsolute (*pageObj));

		emit layoutFinished ();
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
				&PagesLayoutManager::Relayout);
		RelayoutScheduled_ = true;
	}
}
