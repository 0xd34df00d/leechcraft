/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QPainter>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/ihavetoc.h>
#include <interfaces/monocle/isupportpainting.h>
#include <interfaces/monocle/isearchabledocument.h>
#include "monocleutilconfig.h"

class QDomElement;
class QTextDocument;

namespace LC::Monocle
{
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
											   , public IHaveTOC
											   , public ISupportPainting
											   , public ISearchableDocument
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IDocument
				LC::Monocle::IHaveTOC
				LC::Monocle::ISearchableDocument
				LC::Monocle::ISupportPainting)
	protected:
		/** @brief The adapted QTextDocument.
		 */
		std::unique_ptr<QTextDocument> Doc_;

		TOCEntryLevel_t TOC_;

		QMap<int, QList<ILink_ptr>> Links_;
	public:
		struct InternalLink
		{
			QPair<int, int> FromSpan_;
			QPair<int, int> ToSpan_;
			// Double-check Q_DECLARE_TYPEINFO when updating this type.
		};

		using LocatedImage_t = QPair<QString, QImage>;
		using ImagesList_t = QVector<LocatedImage_t>;

		~TextDocumentAdapter () override;

		bool IsValid () const override;
		int GetNumPages () const override;
		QSize GetPageSize (int page) const override;
		QFuture<QImage> RenderPage (int page, double xScale, double yScale) override;
		QList<ILink_ptr> GetPageLinks (int page) override;

		TOCEntryLevel_t GetTOC () override;

		void PaintPage (QPainter *painter, int page, double xScale, double yScale) override;

		QMap<int, QList<QRectF>> GetTextPositions (const QString& text, Qt::CaseSensitivity cs) override;

		/** @brief Sets the underlying document and any additional information.
		 *
		 * @param[in] doc The document.
		 * @param[in] images The images that are referred in `doc`.
		 * @param[in] coverImage One of the `images` to be used as the cover image.
		 */
		void SetDocument (const QDomElement& doc, const ImagesList_t& images, const QString& coverId = {});
	signals:
		void navigateRequested (const QString&, const IDocument::Position&) override;
		void printRequested (const QList<int>&) override;
	};
}

Q_DECLARE_TYPEINFO (LC::Monocle::TextDocumentAdapter::InternalLink, Q_PRIMITIVE_TYPE);
