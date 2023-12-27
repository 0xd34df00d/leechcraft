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
#include <interfaces/monocle/isupportpainting.h>
#include <interfaces/monocle/isearchabledocument.h>
#include "monocleutilconfig.h"

class QTextDocument;

namespace LC
{
namespace Monocle
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
	class MONOCLE_UTIL_API TextDocumentAdapter : public IDocument
											   , public ISupportPainting
											   , public ISearchableDocument
	{
	protected:
		/** @brief The adapted QTextDocument.
		 */
		std::shared_ptr<QTextDocument> Doc_;

		QMap<int, QList<ILink_ptr>> Links_;
	public:
		struct InternalLink
		{
			QPair<int, int> FromSpan_;
			QPair<int, int> ToSpan_;
		};

		/** @brief Constructs the TextDocumentAdapter over the \em document.
		 */
		TextDocumentAdapter (const std::shared_ptr<QTextDocument>& document = {});

		/** @brief Checks if a document is set.
		 */
		bool IsValid () const override;

		/** @brief Returns the pages count in the underlying document.
		 *
		 * @note If IsValid() returns false, the behavior is undefined.
		 *
		 * @return Pages count.
		 */
		int GetNumPages () const override;

		/** @brief Returns the size of the \em page.
		 *
		 * @note If IsValid() returns false, the behavior is undefined.
		 *
		 * @param[in] page The index of the page to query.
		 * @return The size of the given \em page in pixels.
		 */
		QSize GetPageSize (int page) const override;

		/** @brief Renders the given \em page.
		 *
		 * The hints set via SetRenderHint() are used during rendering.
		 *
		 * @note If IsValid() returns false, the behavior is undefined.
		 *
		 * @param[in] page The index of the page to render.
		 * @param[in] xScale The scale in the X dimension.
		 * @param[in] yScale The scale in the Y dimension.
		 *
		 * @return The rendered image of the given page.
		 *
		 * @sa SetRenderHint()
		 */
		QFuture<QImage> RenderPage (int page, double xScale, double yScale) override;

		/** @brief Returns the links found on the given \em page.
		 *
		 * The implementation currently always returns an empty list.
		 *
		 * @note If IsValid() returns false, the behavior is undefined.
		 *
		 * @param[in] page The index of the page to query.
		 *
		 * @return The list of links found on the \em page.
		 */
		QList<ILink_ptr> GetPageLinks (int page) override;

		/** @brief Paints the given \em page using the \em painter.
		 *
		 * The hints set via SetRenderHint() are used during rendering.
		 *
		 * @note If IsValid() returns false, the behavior is undefined.
		 *
		 * @param[in] painter The painter to use.
		 * @param[in] page The index of the page to paint.
		 * @param[in] xScale The X-axis scale which should be used during
		 * painting.
		 * @param[in] yScale The Y-axis scale which should be used during
		 * painting.
		 *
		 * @sa SetRenderHint()
		 */
		void PaintPage (QPainter *painter, int page, double xScale, double yScale) override;

		/** @brief Searches for the given \em text and returns its positions.
		 *
		 * @note If IsValid() returns false, the behavior is undefined.
		 *
		 * @param[in] text The text to search for.
		 * @param[in] cs The case sensitivity of the search.
		 *
		 * @return The list of positions of the given \em text string for
		 * each page.
		 */
		QMap<int, QList<QRectF>> GetTextPositions (const QString& text, Qt::CaseSensitivity cs) override;
	protected:
		/** @brief Sets the underlying document to \em doc.
		 *
		 * @param[in] doc The document to use.
		 */
		void SetDocument (const std::shared_ptr<QTextDocument>& doc, const QList<InternalLink>& links = {});
	};
}
}
