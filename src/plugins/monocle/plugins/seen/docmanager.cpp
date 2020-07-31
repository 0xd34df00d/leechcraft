/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "docmanager.h"
#include <QtDebug>
#include "document.h"

namespace LC
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
		const auto doc = std::make_shared<Document> (file, Context_, parent (), this);
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
		if (const auto& doc = weak.lock ())
			doc->UpdateDocInfo ();
		else
			qWarning () << Q_FUNC_INFO
					<< "document is dead";
	}

	void DocManager::HandlePageInfo (ddjvu_document_t *nativeDoc, ddjvu_page_t *page)
	{
		const auto& weak = Documents_ [nativeDoc];
		if (const auto doc = weak.lock ())
			doc->UpdatePageInfo (page);
		else
			qWarning () << Q_FUNC_INFO
					<< "document is dead";
	}

	void DocManager::RedrawPage (ddjvu_document_t *nativeDoc, ddjvu_page_t *page)
	{
		const auto& weak = Documents_ [nativeDoc];
		if (const auto doc = weak.lock ())
			doc->RedrawPage (page);
		else
			qWarning () << Q_FUNC_INFO
					<< "document is dead";
	}
}
}
}
