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

#include "thumbswidget.h"
#include <QtDebug>
#include "pageslayoutmanager.h"
#include "pagegraphicsitem.h"
#include "common.h"

namespace LeechCraft
{
namespace Monocle
{
	ThumbsWidget::ThumbsWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.ThumbsView_->setScene (&Scene_);
		Ui_.ThumbsView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		LayoutMgr_ = new PagesLayoutManager (Ui_.ThumbsView_, this);
		LayoutMgr_->SetScaleMode (ScaleMode::FitWidth);
		LayoutMgr_->SetMargins (10, 0);

		connect (LayoutMgr_,
				SIGNAL (scheduledRelayoutFinished ()),
				this,
				SLOT (handleRelayouted ()));
	}

	void ThumbsWidget::HandleDoc (IDocument_ptr doc)
	{
		Scene_.clear ();
		CurrentAreaRects_.clear ();
		CurrentDoc_ = doc;

		if (!doc)
			return;

		QList<PageGraphicsItem*> pages;
		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);
			item->SetReleaseHandler ([this] (int page, const QPointF&) { emit pageClicked (page); });
			pages << item;
		}

		LayoutMgr_->HandleDoc (CurrentDoc_, pages);
		LayoutMgr_->Relayout ();
	}

	void ThumbsWidget::updatePagesVisibility (const QMap<int, QRect>& page2rect)
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

		const auto& pages = LayoutMgr_->GetPages ();

		int rectIdx = 0;
		for (auto i = page2rect.begin (); i != page2rect.end (); ++i, ++rectIdx)
		{
			const auto pageNum = i.key ();
			if (pageNum >= pages.size ())
				continue;

			auto page = pages.at (pageNum);

			const auto& docRect = *i;
			const auto& sceneRect = page->mapToScene (page->MapFromDoc (docRect)).boundingRect ();
			CurrentAreaRects_ [rectIdx]->setRect (sceneRect);
		}
	}

	void ThumbsWidget::handleCurrentPage (int page)
	{
		LayoutMgr_->SetCurrentPage (page, false);
	}

	void ThumbsWidget::handleRelayouted ()
	{
		updatePagesVisibility (LastVisibleAreas_);
	}
}
}

