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
#include <QUrl>
#include <util/monocle/textdocumentadapter.h>

namespace LC
{
namespace Monocle
{
namespace Dik
{
	class MobiParser;

	class Document : public QObject
				   , public TextDocumentAdapter
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IDocument
				LC::Monocle::ISearchableDocument
				LC::Monocle::ISupportPainting)

		DocumentInfo Info_;
		QUrl DocURL_;

		std::shared_ptr<MobiParser> Parser_;

		QObject *Plugin_;
	public:
		Document (const QString&, QObject*);

		QObject* GetBackendPlugin () const;
		QObject* GetQObject ();
		DocumentInfo GetDocumentInfo () const;
		QUrl GetDocURL () const;
	signals:
		void navigateRequested (const QString&, const IDocument::Position&);
		void printRequested (const QList<int>&);
	};
}
}
}

