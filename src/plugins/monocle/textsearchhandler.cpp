/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textsearchhandler.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "interfaces/monocle/isearchabledocument.h"
#include "pagegraphicsitem.h"
#include "pageslayoutmanager.h"

namespace LC
{
namespace Monocle
{
	TextSearchHandler::TextSearchHandler (QGraphicsView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scene_ (view->scene ())
	, CurrentRectIndex_ (-1)
	{
	}

	void TextSearchHandler::HandleDoc (IDocument_ptr doc, const QVector<PageGraphicsItem*>& pages)
	{
		Doc_ = doc;
		Pages_ = pages;

		CurrentHighlights_.clear ();
		CurrentRectIndex_ = -1;
		CurrentSearchString_.clear ();
	}

	bool TextSearchHandler::Search (const QString& text, Util::FindNotification::FindFlags flags)
	{
		if (!Doc_)
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
		CurrentSearchString_ = text;

		const auto searchable = qobject_cast<ISearchableDocument*> (Doc_->GetQObject ());
		if (!searchable)
			return false;

		const auto cs = flags & Util::FindNotification::FindCaseSensitively ?
				Qt::CaseSensitive :
				Qt::CaseInsensitive;
		const auto& map = searchable->GetTextPositions (text, cs);
		emit gotSearchResults ({ text, flags, map });

		BuildHighlights (map);

		if (!CurrentHighlights_.isEmpty ())
			SelectItem (0);

		return !CurrentHighlights_.isEmpty ();
	}

	void TextSearchHandler::BuildHighlights (const QMap<int, QList<QRectF>>& map)
	{
		const QBrush brush (Qt::yellow);
		for (const auto& pair : Util::Stlize (map))
		{
			const auto page = Pages_.at (pair.first);
			for (const auto& rect : pair.second)
			{
				const auto item = new QGraphicsRectItem (page);
				item->setBrush (brush);
				item->setZValue (1);
				item->setOpacity (0.2);
				CurrentHighlights_ << item;

				page->RegisterChildRect (item, rect,
						[item] (const QRectF& rect) { item->setRect (rect); });
			}
		}
	}

	void TextSearchHandler::ClearHighlights ()
	{
		for (auto item : CurrentHighlights_)
		{
			auto parentPage = static_cast<PageGraphicsItem*> (item->parentItem ());
			parentPage->UnregisterChildRect (item);
			Scene_->removeItem (item);
			delete item;
		}

		CurrentHighlights_.clear ();
	}

	void TextSearchHandler::SelectItem (int index)
	{
		if (CurrentRectIndex_ >= 0 && CurrentRectIndex_ < CurrentHighlights_.size ())
		{
			auto oldHili = CurrentHighlights_.at (CurrentRectIndex_);
			oldHili->setOpacity (0.2);
			oldHili->setPen (QPen ());
		}

		auto item = CurrentHighlights_.at (index);
		item->setOpacity (0.6);
		item->setPen ({ Qt::black });
		CurrentRectIndex_ = index;

		auto pageItem = static_cast<PageGraphicsItem*> (item->parentItem ());
		const auto pageIdx = pageItem->GetPageNum ();
		const auto& bounding = pageItem->boundingRect ();

		auto rect = item->rect ();
		rect.setLeft (rect.left () / bounding.width ());
		rect.setTop (rect.top () / bounding.height ());
		rect.setWidth (rect.width () / bounding.width ());
		rect.setHeight (rect.height () / bounding.height ());
		emit navigateRequested ({ pageIdx, rect });
	}
}
}
