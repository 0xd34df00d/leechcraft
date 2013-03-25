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

#include "textsearchhandler.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QtDebug>
#include "interfaces/monocle/isearchabledocument.h"
#include "pagegraphicsitem.h"
#include "pageslayoutmanager.h"

namespace LeechCraft
{
namespace Monocle
{
	TextSearchHandler::TextSearchHandler (QGraphicsView *view, PagesLayoutManager *mgr, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scene_ (view->scene ())
	, LayoutMgr_ (mgr)
	, CurrentRectIndex_ (-1)
	{
	}

	void TextSearchHandler::HandleDoc (IDocument_ptr doc, const QList<PageGraphicsItem*>& pages)
	{
		Doc_ = doc;
		Pages_ = pages;

		CurrentHighlights_.clear ();
		CurrentRectIndex_ = -1;
		CurrentSearchString_.clear ();
	}

	void TextSearchHandler::Search (const QString& text, Util::FindNotification::FindFlags flags)
	{
		if (!Doc_)
			return;

		if (text != CurrentSearchString_)
		{
			CurrentSearchString_ = text;
			for (auto item : CurrentHighlights_)
			{
				auto parentPage = static_cast<PageGraphicsItem*> (item->parentItem ());
				parentPage->UnregisterChildRect (item);
				Scene_->removeItem (item);
				delete item;
			}

			CurrentHighlights_.clear ();

			auto searchable = qobject_cast<ISearchableDocument*> (Doc_->GetQObject ());
			if (!searchable)
				return;

			const QBrush brush (Qt::yellow);

			auto cs = flags & Util::FindNotification::FindCaseSensitively ?
					Qt::CaseSensitive :
					Qt::CaseInsensitive;
			const auto& map = searchable->GetTextPositions (text, cs);
			for (auto i = map.begin (); i != map.end (); ++i)
			{
				auto page = Pages_.at (i.key ());
				for (const auto& rect : *i)
				{
					auto item = new QGraphicsRectItem (page);
					item->setBrush (brush);
					item->setZValue (1);
					item->setOpacity (0.2);
					CurrentHighlights_ << item;

					page->RegisterChildRect (item, rect);
				}
			}

			if (!CurrentHighlights_.isEmpty ())
				SelectItem (0);

			return;
		}

		if (CurrentHighlights_.isEmpty ())
			return;

		if (flags & Util::FindNotification::FindBackwards)
		{
			auto nextIdx = CurrentRectIndex_ - 1;
			if (nextIdx < 0)
			{
				if (flags & Util::FindNotification::FindWrapsAround)
					nextIdx = CurrentHighlights_.size () - 1;
				else
					return;
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
					return;
			}
			SelectItem (nextIdx);
		}
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
		const auto pageIdx = LayoutMgr_->GetPages ().indexOf (pageItem);
		if (pageIdx >= 0)
			LayoutMgr_->SetCurrentPage (pageIdx, false);
	}
}
}
