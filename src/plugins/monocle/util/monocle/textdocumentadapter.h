/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QDomElement>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/ihavetextcontent.h>
#include <interfaces/monocle/ihavetoc.h>
#include <interfaces/monocle/isupportpainting.h>
#include <interfaces/monocle/isearchabledocument.h>
#include "monocleutilconfig.h"
#include "types.h"

class QDomElement;
class QPainter;

namespace LC::Monocle
{
	class ResourcedTextDocument;

	/** @brief Provides an adapter of QTextDocument to Monocle's IDocument.
	 *
	 * This class provides implementations for most of the IDocument's
	 * methods, as well as methods of ISupportPainting and
	 * ISearchableDocument, working over a QTextDocument.
	 *
	 * The document for this class to work on is passed either via the
	 * constructor or by calling the SetDocument() method. The adapter
	 * owns the document.
	 */
	class MONOCLE_UTIL_API TextDocumentAdapter : public QObject
											   , public IDocument
											   , public IHaveTextContent
											   , public IHaveTOC
											   , public ISupportPainting
											   , public ISearchableDocument
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IDocument
				LC::Monocle::IHaveTextContent
				LC::Monocle::IHaveTOC
				LC::Monocle::ISearchableDocument
				LC::Monocle::ISupportPainting)
	protected:
		std::unique_ptr<ResourcedTextDocument> Doc_;
		TOCEntryLevel_t TOC_;
		QHash<int, QList<ILink_ptr>> Links_;
	public:
		~TextDocumentAdapter () override;

		QObject* GetQObject () override;
		bool IsValid () const override;
		int GetNumPages () const override;
		QSize GetPageSize (int page) const override;
		QFuture<QImage> RenderPage (int page, double xScale, double yScale) override;
		QList<ILink_ptr> GetPageLinks (int page) override;
		const DocumentSignals* GetDocumentSignals () const override;

		QString GetTextContent (int page, const PageRelativeRectBase& rect) override;

		TOCEntryLevel_t GetTOC () override;

		void PaintPage (QPainter *painter, int page, double xScale, double yScale) override;

		QMap<int, QList<PageRelativeRectBase>> GetTextPositions (const QString& text, Qt::CaseSensitivity cs) override;

		struct HtmlDocument
		{
			QDomElement BodyElem_;
			TOCEntryID TocStructure_;
			LazyImages_t Images_;
			QString CoverId_ = {};

			CustomStyler_f Styler_ = {};
		};

		void SetDocument (const HtmlDocument&);
	};
}
