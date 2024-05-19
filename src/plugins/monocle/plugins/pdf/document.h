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
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/ihavetoc.h>
#include <interfaces/monocle/ihavetextcontent.h>
#include <interfaces/monocle/ihavefontinfo.h>
#include <interfaces/monocle/isupportannotations.h>
#include <interfaces/monocle/isupportforms.h>
#include <interfaces/monocle/isearchabledocument.h>
#include <interfaces/monocle/isaveabledocument.h>
#include <interfaces/monocle/isupportpainting.h>
#include <interfaces/monocle/ihaveoptionalcontent.h>
#include <util/monocle/documentsignals.h>

namespace Poppler
{
	class Document;
}

namespace LC
{
namespace Monocle
{
namespace PDF
{
	typedef std::shared_ptr<Poppler::Document> PDocument_ptr;

	class Document : public QObject
				   , public IDocument
				   , public IHaveTOC
				   , public IHaveTextContent
				   , public IHaveOptionalContent
				   , public IHaveFontInfo
				   , public ISupportAnnotations
				   , public ISupportForms
				   , public ISupportPainting
				   , public ISearchableDocument
				   , public ISaveableDocument
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IDocument
				LC::Monocle::IHaveTOC
				LC::Monocle::IHaveTextContent
				LC::Monocle::IHaveOptionalContent
				LC::Monocle::IHaveFontInfo
				LC::Monocle::ISupportAnnotations
				LC::Monocle::ISupportForms
				LC::Monocle::ISupportPainting
				LC::Monocle::ISearchableDocument
				LC::Monocle::ISaveableDocument)

		PDocument_ptr PDocument_;
		TOCEntryLevel_t TOC_;
		QUrl DocURL_;

		QObject *Plugin_;

		DocumentSignals Signals_;
	public:
		Document (const QString&, QObject*);

		QObject* GetBackendPlugin () const;
		QObject* GetQObject ();
		bool IsValid () const;
		DocumentInfo GetDocumentInfo () const;
		int GetNumPages () const;
		QSize GetPageSize (int) const;
		QFuture<QImage> RenderPage (int, double, double);
		QList<ILink_ptr> GetPageLinks (int);
		QUrl GetDocURL () const;
		const DocumentSignals* GetDocumentSignals () const;

		TOCEntryLevel_t GetTOC ();

		QString GetTextContent (int, const QRect&);

		QAbstractItemModel* GetOptContentModel ();

		IPendingFontInfoRequest* RequestFontInfos () const;

		QList<IAnnotation_ptr> GetAnnotations (int);

		IFormFields_t GetFormFields (int);

		void PaintPage (QPainter*, int, double, double);

		QMap<int, QList<QRectF>> GetTextPositions (const QString&, Qt::CaseSensitivity);

		SaveQueryResult CanSave () const;
		bool Save (const QString& path);

		void RequestPrinting ();
	private:
		void BuildTOC ();
	};
}
}
}
