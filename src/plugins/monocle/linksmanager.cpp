/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linksmanager.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include "documenttab.h"
#include "linkitem.h"
#include "pagegraphicsitem.h"

namespace LC
{
namespace Monocle
{
	LinksManager::LinksManager (DocumentTab& docTab)
	: QObject { &docTab }
	, DocTab_ { docTab }
	{
	}

	void LinksManager::HandleDoc (IDocument_ptr doc, const QList<PageGraphicsItem*>& pages)
	{
		for (auto page : pages)
			for (const auto& link : doc->GetPageLinks (page->GetPageNum ()))
			{
				auto item = new LinkItem (link, page, DocTab_);
				page->RegisterChildRect (item, link->GetArea (),
						[item] (const QRectF& rect) { item->setRect (rect); });
			}
	}
}
}
