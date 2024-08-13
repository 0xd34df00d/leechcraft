/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "thumbswidget.h"
#include <QtDebug>
#include "interfaces/monocle/idocument.h"
#include "components/services/pixmapcachemanager.h"
#include "components/viewitems/pagegraphicsitem.h"
#include "pageslayoutmanager.h"
#include "smoothscroller.h"
#include "common.h"

namespace LC::Monocle
{
	ThumbsWidget::ThumbsWidget (PixmapCacheManager& pxCache, QWidget *parent)
	: QWidget { parent }
	, PxCache_ { pxCache }
	, Scroller_ { *new SmoothScroller { *Ui_.ThumbsView_, this } }
	{
		Ui_.ThumbsView_->setScene (&Scene_);
		Ui_.ThumbsView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		LayoutMgr_ = new PagesLayoutManager (Ui_.ThumbsView_, this);
		LayoutMgr_->SetScaleMode (FitWidth {});
		LayoutMgr_->SetMargins ({ 10, 0 });

		connect (LayoutMgr_,
				&PagesLayoutManager::layoutFinished,
				this,
				[this] { UpdatePagesVisibility (LastVisibleAreas_); });
	}

	void ThumbsWidget::HandleDoc (IDocument& doc)
	{
		Scene_.clear ();
		Pages_.clear ();
		CurrentAreaRects_.clear ();

		const auto numPages = doc.GetNumPages ();
		Pages_.reserve (numPages);
		for (int i = 0; i < numPages; ++i)
		{
			auto item = new PageGraphicsItem { doc, i };
			PxCache_.RegisterPage (*item);
			Scene_.addItem (item);
			item->SetReleaseHandler ([this] (int page, auto&&) { emit pageClicked (page); });
			Pages_ << item;
		}

		LayoutMgr_->HandleDoc (&doc, Pages_);
		LayoutMgr_->Relayout ();
	}

	void ThumbsWidget::UpdatePagesVisibility (const QMap<int, SceneAbsoluteRect>& page2rect)
	{
		LastVisibleAreas_ = page2rect;

		if (page2rect.size () != CurrentAreaRects_.size ())
		{
			for (auto rect : CurrentAreaRects_)
			{
				Scene_.removeItem (rect);
				delete rect;
			}
			CurrentAreaRects_.clear ();

			const auto& brush = palette ().brush (QPalette::Dark);
			for (int i = 0; i < page2rect.size (); ++i)
			{
				auto item = Scene_.addRect ({}, { Qt::black }, brush);
				item->setZValue (1);
				item->setOpacity (0.3);
				CurrentAreaRects_ << item;
			}
		}

		int rectIdx = 0;
		for (auto i = page2rect.begin (); i != page2rect.end (); ++i, ++rectIdx)
			CurrentAreaRects_ [rectIdx]->setRect (i->ToRectF ());
	}

	void ThumbsWidget::SetCurrentPage (int page)
	{
		Scroller_.SmoothCenterOn (*Pages_ [page]);
	}
}
