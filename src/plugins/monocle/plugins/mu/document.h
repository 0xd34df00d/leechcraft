/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>

extern "C"
{
#include <mupdf.h>
}

#include <interfaces/monocle/idocument.h>

namespace LC
{
namespace Monocle
{
namespace Mu
{
	class Document : public QObject
				   , public IDocument
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IDocument)

		fz_context *MuCtx_;
		pdf_document *MuDoc_;

		QUrl URL_;

		QObject *Plugin_;
	public:
		Document (const QString&, fz_context*, QObject*);
		~Document ();

		QObject* GetBackendPlugin () const;
		QObject* GetQObject ();
		bool IsValid () const;
		DocumentInfo GetDocumentInfo () const;
		int GetNumPages () const;
		QSize GetPageSize (int) const;
		QImage RenderPage (int, double xRes, double yRes);
		QList<ILink_ptr> GetPageLinks (int);
		QUrl GetDocURL () const;
	signals:
		void navigateRequested (const QString& , const IDocument::Position&);
		void printRequested (const QList<int>&);
	};
}
}
}
