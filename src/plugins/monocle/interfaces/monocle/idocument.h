/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QImage>
#include <QMetaType>
#include <QStringList>
#include <QDateTime>
#include "ilink.h"

class QUrl;

template<typename>
class QFuture;

namespace LC
{
namespace Monocle
{
	/** @brief Document metadata.
	 *
	 * All fields in this structure can be null.
	 */
	struct DocumentInfo
	{
		/** @brief Document title.
		 */
		QString Title_;
		/** @brief The subject line of this document.
		 */
		QString Subject_;
		/** @brief Description of the document.
		 */
		QString Description_;
		/** @brief The author of the document.
		 */
		QString Author_;

		/** @brief Genres of this document.
		 */
		QStringList Genres_;
		/** @brief Keywords corresponding to this document.
		 */
		QStringList Keywords_;

		/** @brief Date this document was created.
		 */
		QDateTime Date_;
	};

	/** @brief Basic interface for documents.
	 *
	 * This interface is the basic interface for documents returned from
	 * format backends.
	 *
	 * Pages actions (like rendering) are also performed via this
	 * interface. Pages indexes are zero-based.
	 *
	 * This class has some signals, and one can use the GetQObject()
	 * method to get an object of this class as a QObject and connect to
	 * those signals.
	 *
	 * There are also other interfaces for extended functionality like
	 * documents that can be changed and saved, the documents containing
	 * annotations and so on. See the <em>See also</em> section for the
	 * details.
	 *
	 * @sa IBackendPlugin::LoadDocument()
	 * @sa IDynamicDocument, IHaveTextContent, ISaveableDocument
	 * @sa ISearchableDocument, ISupportAnnotations, ISupportForms
	 * @sa IHaveTOC, ISupportPainting
	 */
	class IDocument
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IDocument () {}

		/** @brief Returns the parent backend plugin.
		 *
		 * This function should return the instance object of the backend
		 * plugin that created this document.
		 *
		 * The returned value should obviously implement IBackendPlugin.
		 *
		 * @return The parent backend plugin instance object.
		 */
		virtual QObject* GetBackendPlugin () const = 0;

		/** @brief Returns this object as a QObject.
		 *
		 * This function can be used to connect to the signals of this
		 * class.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns whether this document is valid.
		 *
		 * An invalid document is basically equivalent to a null pointer,
		 * all operations on it lead to undefined behavior.
		 *
		 * @return Whether this document is valid.
		 */
		virtual bool IsValid () const = 0;

		/** @brief Returns the document metadata.
		 *
		 * @return The document metadata.
		 */
		virtual DocumentInfo GetDocumentInfo () const = 0;

		/** @brief Returns the number of pages in this document.
		 *
		 * @return The number of pages in this document.
		 */
		virtual int GetNumPages () const = 0;

		/** @brief Returns the size in points of the given page.
		 *
		 * This function returns the physical dimensions of the given
		 * \em page in points.
		 *
		 * Some formats support different pages having different sizes in
		 * the same document, thus the size should be queried for each
		 * \em page.
		 *
		 * @param[in] page The index of the page to query.
		 * @return The size of the given \em page.
		 */
		virtual QSize GetPageSize (int page) const = 0;

		/** @brief Renders the given \em page at the given scale.
		 *
		 * This function should return an image with the given \em page
		 * rendered at the given \em xScale and \em yScale for <em>x</em>
		 * and <em>y</em> axises correspondingly. That is, the image's
		 * size should be equal to the following:
		 * \code
			auto size = GetPageSize (page);
			size.rwidth () *= xScale;
			size.rheight () *= yscale;
		   \endcode
		 *
		 * @param[in] page The index of the page to render.
		 * @param[in] xScale The scale of the <em>x</em> axis.
		 * @param[in] yScale The scale of the <em>y</em> axis.
		 * @return The rendering of the given page.
		 */
		virtual QFuture<QImage> RenderPage (int page, double xScale, double yScale) = 0;

		/** @brief Returns the links found at the given \em page.
		 *
		 * If the format doesn't support links, an empty list should be
		 * returned.
		 *
		 * The ownership of the returned links objects is passed to the
		 * caller.
		 *
		 * @param[in] page The page index to query.
		 *
		 * @sa ILink
		 */
		virtual QList<ILink_ptr> GetPageLinks (int page) = 0;

		/** @brief Returns the URL of the document.
		 *
		 * This method should return the URL of this document. URLs on
		 * the local filesystem should obviously have the <em>file</em>
		 * scheme.
		 *
		 * @return The URL of this document.
		 */
		virtual QUrl GetDocURL () const = 0;

		/** @brief Emitted when printing is requested.
		 *
		 * This signal is emitted when printing is requested, for
		 * example, by a link action.
		 *
		 * @param[out] pages The list of pages to print, or an empty list
		 * to print all pages.
		 */
		virtual void printRequested (const QList<int>& pages) = 0;
	};

	/** @brief Shared pointer to a document.
	 */
	typedef std::shared_ptr<IDocument> IDocument_ptr;
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IDocument,
		"org.LeechCraft.Monocle.IDocument/1.0")
