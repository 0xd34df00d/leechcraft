/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewpositiontracker.h"
#include <QScrollBar>
#include "components/viewitems/pagegraphicsitem.h"
#include "pageslayoutmanager.h"
#include "pagesview.h"

namespace LC::Monocle
{
	ViewPositionTracker::ViewPositionTracker (PagesView& view, PagesLayoutManager& layoutManager, QObject *parent)
	: QObject { parent }
	, LayoutManager_ { layoutManager }
	, View_ { view }
	{
		connect (view.verticalScrollBar (),
				&QScrollBar::valueChanged,
				this,
				&ViewPositionTracker::Update);
		connect (&layoutManager,
				&PagesLayoutManager::layoutFinished,
				this,
				&ViewPositionTracker::Update);
	}

	void ViewPositionTracker::SetUpdatesEnabled (bool enabled)
	{
		if (UpdatesEnabled_ == enabled)
			return;

		UpdatesEnabled_ = enabled;
		if (enabled)
			Update ();
	}

	void ViewPositionTracker::Update ()
	{
		if (!UpdatesEnabled_)
			return;

		RegenPageVisibility ();

		auto current = LayoutManager_.GetCurrentPage ();
		if (PrevCurrentPage_ != current)
		{
			PrevCurrentPage_ = current;
			emit currentPageChanged (current);
		}
	}

	void ViewPositionTracker::RegenPageVisibility ()
	{
		const auto& visibleRect = ViewAbsoluteRect { View_ }.ToSceneAbsolute (View_);

		QMap<int, PageRelativeRect> rects;
		for (auto item : View_.scene ()->items (visibleRect.ToRectF ()))
		{
			auto page = dynamic_cast<PageGraphicsItem*> (item);
			if (!page)
				continue;

			const auto& pageRect = PageAbsoluteRect { page->boundingRect () }.ToPageRelative (*page);
			rects [page->GetPageNum ()] = visibleRect.ToPageRelative (*page) & pageRect;
		}

		emit pagesVisibilityChanged (rects);
	}
}
