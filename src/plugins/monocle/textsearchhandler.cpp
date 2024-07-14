/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textsearchhandler.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "interfaces/monocle/isearchabledocument.h"
#include "components/layout/positions.h"
#include "pagegraphicsitem.h"

namespace LC::Monocle
{
	void TextSearchHandler::HandleDoc (IDocument& doc, const QVector<PageGraphicsItem*>& pages)
	{
		CurrentHighlights_.clear ();
		CurrentRectIndex_ = -1;
		CurrentSearchString_.clear ();
		Pages_ = pages;

		SearchableDoc_ = qobject_cast<ISearchableDocument*> (doc.GetQObject ());
	}

	bool TextSearchHandler::Search (const QString& text, Util::FindNotification::FindFlags flags)
	{
		if (!SearchableDoc_)
			return false;

		if (text != CurrentSearchString_)
			return RequestSearch (text, flags);

		if (CurrentHighlights_.isEmpty ())
			return false;

		if (flags & Util::FindNotification::FindBackwards)
		{
			auto nextIdx = CurrentRectIndex_ - 1;
			if (nextIdx < 0)
			{
				if (flags & Util::FindNotification::FindWrapsAround)
					nextIdx = CurrentHighlights_.size () - 1;
				else
					return false;
			}
			SelectItem (nextIdx);
		}
		else
		{
			auto nextIdx = CurrentRectIndex_ + 1;
			if (nextIdx >= CurrentHighlights_.size ())
			{
				if (flags & Util::FindNotification::FindWrapsAround)
					nextIdx = 0;
				else
					return false;
			}
			SelectItem (nextIdx);
		}

		return true;
	}

	void TextSearchHandler::SetPreparedResults (const TextSearchHandlerResults& results, int select)
	{
		if (CurrentSearchString_ != results.Text_)
		{
			ClearHighlights ();
			CurrentSearchString_ = results.Text_;
			BuildHighlights (results.Positions_);
		}

		SelectItem (select);
	}

	bool TextSearchHandler::RequestSearch (const QString& text, Util::FindNotification::FindFlags flags)
	{
		ClearHighlights ();

		if (!SearchableDoc_)
			return false;

		CurrentSearchString_ = text;
		const auto cs = flags & Util::FindNotification::FindCaseSensitively ?
				Qt::CaseSensitive :
				Qt::CaseInsensitive;
		const auto& map = SearchableDoc_->GetTextPositions (text, cs);
		emit gotSearchResults ({ text, flags, map });

		BuildHighlights (map);

		if (!CurrentHighlights_.isEmpty ())
			SelectItem (0);

		return !CurrentHighlights_.isEmpty ();
	}

	namespace
	{
		constexpr double InactiveOpacity = 0.2;
		constexpr double ActiveOpacity = 0.6;
	}

	void TextSearchHandler::BuildHighlights (const QMap<int, QList<PageRelativeRectBase>>& map)
	{
		const QBrush brush (Qt::yellow);
		for (const auto& [pageIdx, rects] : Util::Stlize (map))
		{
			const auto page = Pages_ [pageIdx];
			for (const auto& rect : rects)
			{
				const auto item = new QGraphicsRectItem (page);
				item->setBrush (brush);
				item->setZValue (1);
				item->setOpacity (InactiveOpacity);
				CurrentHighlights_.push_back ({ item, rect, pageIdx });

				page->RegisterChildRect (item, rect,
						[item] (const PageAbsoluteRect& rect) { item->setRect (rect.ToRectF ()); });
			}
		}
	}

	void TextSearchHandler::ClearHighlights ()
	{
		for (const auto& highlight : CurrentHighlights_)
		{
			const auto item = highlight.Item_;
			auto& parentPage = dynamic_cast<PageGraphicsItem&> (*item->parentItem ());
			parentPage.UnregisterChildRect (item);
			delete item;
		}

		CurrentHighlights_.clear ();
	}

	void TextSearchHandler::SelectItem (int index)
	{
		if (CurrentRectIndex_ >= 0 && CurrentRectIndex_ < CurrentHighlights_.size ())
		{
			auto oldHili = CurrentHighlights_.at (CurrentRectIndex_).Item_;
			oldHili->setOpacity (InactiveOpacity);
			oldHili->setPen ({});
		}

		auto [item, rect, pageIdx] = CurrentHighlights_.at (index);
		item->setOpacity (ActiveOpacity);
		item->setPen ({ Qt::black });
		CurrentRectIndex_ = index;

		emit navigateRequested ({ pageIdx, rect });
	}
}
