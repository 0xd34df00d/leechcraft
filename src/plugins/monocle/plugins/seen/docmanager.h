/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <interfaces/monocle/ibackendplugin.h>
#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>

namespace LC
{
namespace Monocle
{
namespace Seen
{
	class Document;

	class DocManager : public QObject
	{
		Q_OBJECT

		ddjvu_context_t *Context_;
		QHash<ddjvu_document_t*, std::weak_ptr<Document>> Documents_;
	public:
		DocManager (ddjvu_context_t*, QObject*);

		std::shared_ptr<Document> LoadDocument (const QString&);
		void Unregister (ddjvu_document_t*);

		void HandleDocInfo (ddjvu_document_t*);
		void HandlePageInfo (ddjvu_document_t*, ddjvu_page_t*);
		void RedrawPage (ddjvu_document_t*, ddjvu_page_t*);
	};
}
}
}
