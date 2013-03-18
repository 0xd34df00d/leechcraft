/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "pageslayoutmanager.h"
#include <QGraphicsScene>
#include <QScrollBar>
#include <QTimer>
#include <QtDebug>
#include "interfaces/monocle/idynamicdocument.h"
#include "pagesview.h"
#include "pagegraphicsitem.h"
#include "common.h"

namespace LeechCraft
{
namespace Monocle
{
	const int Margin = 10;

	PagesLayoutManager::PagesLayoutManager (PagesView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scene_ (view->scene ())
	, LayMode_ (LayoutMode::OnePage)
	, ScaleMode_ (ScaleMode::FitWidth)
	, FixedScale_ (1)
	, RelayoutScheduled_ (true)
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

		if (qobject_cast<IDynamicDocument*> (CurrentDoc_->GetQObject ()))
		{
			connect (CurrentDoc_->GetQObject (),
					SIGNAL (pageSizeChanged (int)),
					this,
					SLOT (handlePageSizeChanged (int)));
			connect (CurrentDoc_->GetQObject (),
					SIGNAL (pageContentsChanged (int)),
					this,
					SLOT (handlePageContentsChanged (int)));
		}
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

	int PagesLayoutManager::GetCurrentPage() const
	{
		const auto& center = GetViewportCenter ();
		auto item = View_->itemAt (center - QPoint (1, 1));
		if (!item)
			item = View_->itemAt (center - QPoint (10, 10));
		auto pos = std::find_if (Pages_.begin (), Pages_.end (),
				[item] (decltype (Pages_.front ()) e) { return e == item; });
		return pos == Pages_.end () ? -1 : std::distance (Pages_.begin (), pos);
	}

	void PagesLayoutManager::SetCurrentPage (int idx, bool immediate)
	{
		if (idx < 0 || idx >= Pages_.size ())
			return;

		auto page = Pages_.at (idx);
		const auto& rect = page->boundingRect ();
		const auto& pos = page->scenePos ();
		int xCenter = pos.x () + rect.width () / 2;
		const auto visibleHeight = std::min (static_cast<int> (rect.height ()),
				View_->viewport ()->contentsRect ().height ());
		int yCenter = pos.y () + visibleHeight / 2;

		if (immediate)
			View_->centerOn (xCenter, yCenter);
		else
			View_->SmoothCenterOn (xCenter, yCenter);
	}

	void PagesLayoutManager::SetScaleMode (ScaleMode mode)
	{
		ScaleMode_ = mode;
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

			double dim = dimGetter (CurrentDoc_->GetPageSize (pageIdx));
			auto size = View_->maximumViewportSize ();
			size.rwidth () -= View_->verticalScrollBar ()->size ().width ();
			size.rheight () -= View_->horizontalScrollBar ()->size ().height ();

			const int margin = 3;
			size.rwidth () -= 2 * margin;
			size.rheight () -= 2 * margin;

			return dimGetter (size) / dim;
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

		qWarning () << Q_FUNC_INFO
				<< "unknown scale mode"
				<< static_cast<int> (ScaleMode_);
		return 1;
	}

	void PagesLayoutManager::Relayout ()
	{
		RelayoutScheduled_ = false;

		const auto scale = GetCurrentScale ();
		const auto pageWas = GetCurrentPage ();

		for (auto item : Pages_)
			item->SetScale (scale, scale);

		for (int i = 0, pagesCount = Pages_.size (); i < pagesCount; ++i)
		{
			const auto& size = CurrentDoc_->GetPageSize (i) * scale;
			auto page = Pages_ [i];
			switch (LayMode_)
			{
			case LayoutMode::OnePage:
				page->setPos (0, Margin + (size.height () + Margin) * i);
				break;
			case LayoutMode::TwoPages:
				page->setPos ((i % 2) * (Margin / 3 + size.width ()), Margin + (size.height () + Margin) * (i / 2));
				break;
			}
		}

		Scene_->setSceneRect (Scene_->itemsBoundingRect ());

		SetCurrentPage (std::max (pageWas, 0), true);
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
	}

	void PagesLayoutManager::handlePageSizeChanged (int)
	{
		scheduleRelayout ();
	}
}
}
