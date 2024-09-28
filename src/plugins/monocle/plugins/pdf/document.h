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
	class Page;
}

namespace LC::Monocle::PDF
{
	using PDocument_ptr = std::shared_ptr<Poppler::Document>;

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

		QObject* GetBackendPlugin () const override;
		QObject* GetQObject () override;
		bool IsValid () const override;
		DocumentInfo GetDocumentInfo () const override;
		int GetNumPages () const override;
		QSize GetPageSize (int) const override;
		QFuture<QImage> RenderPage (int, double, double) override;
		QList<ILink_ptr> GetPageLinks (int) override;
		QUrl GetDocURL () const override;
		const DocumentSignals* GetDocumentSignals () const override;

		TOCEntryLevel_t GetTOC () override;

		QString GetTextContent (int, const PageRelativeRectBase&) override;
		QVector<TextBox> GetTextBoxes (int) override;

		QAbstractItemModel* GetOptContentModel () override;

		Util::Task<QList<FontInfo>> RequestFontInfos () const override;

		QList<IAnnotation_ptr> GetAnnotations (int) override;

		IFormFields_t GetFormFields (int) override;

		void PaintPage (QPainter*, int, double, double) override;

		QMap<int, QList<PageRelativeRectBase>> GetTextPositions (const QString&, Qt::CaseSensitivity) override;

		SaveQueryResult CanSave () const override;
		bool Save (const QString& path) override;

		void RequestPrinting ();
	private:
		std::unique_ptr<Poppler::Page> GetPage (int) const;

		void BuildTOC ();
	};
}
