/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "docmanager.h"
#include "document.h"

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	DocManager::DocManager (ddjvu_context_t *ctx, QObject *parent)
	: QObject (parent)
	, Context_ (ctx)
	{
	}

	std::shared_ptr<Document> DocManager::LoadDocument (const QString& file)
	{
		std::shared_ptr<Document> doc (new Document (file, Context_, this));
		Documents_ [doc->GetNativeDoc ()] = doc;
		return doc;
	}

	void DocManager::Unregister (ddjvu_document_t *doc)
	{
		Documents_.remove (doc);
	}

	void DocManager::HandleDocInfo (ddjvu_document_t *nativeDoc)
	{
		auto weak = Documents_ [nativeDoc];
		auto doc = weak.lock ();
		if (doc)
			doc->UpdateDocInfo ();
	}

	void DocManager::HandlePageInfo (ddjvu_document_t *nativeDoc, ddjvu_page_t *page)
	{
		auto weak = Documents_ [nativeDoc];
		auto doc = weak.lock ();
		if (doc)
			doc->UpdatePageInfo (page);
	}

	void DocManager::RedrawPage (ddjvu_document_t *nativeDoc, ddjvu_page_t *page)
	{
		auto weak = Documents_ [nativeDoc];
		auto doc = weak.lock ();
		if (doc)
			doc->RedrawPage (page);
	}
}
}
}
