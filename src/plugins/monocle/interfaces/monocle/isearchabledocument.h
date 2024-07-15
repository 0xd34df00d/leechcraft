/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QtPlugin>
#include "coords.h"

namespace LC
{
namespace Monocle
{
	/** @brief Interface for documents supporting searching for text.
	 * 
	 * If document represents a format that supports searching for text,
	 * the document should implement this interface.
	 * 
	 * @sa IDocument
	 */
	class ISearchableDocument
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~ISearchableDocument () {}

		/** @brief Returns the search results for the \em text.
		 * 
		 * This function should return the map where keys are indexes of
		 * pages containing the given \em text and with each value
		 * corresponding to a key being a list of rectangles containing
		 * the \em text on the page.
		 *
		 * If a page doesn't contain any occurrences of \em text it
		 * should better be omitted from the map for performance reasons.
		 * 
		 * @param[in] text The text to search for.
		 * @param[in] cs The case sensitivity of the search.
		 * @return The map from page indexes to list of rectangles
		 * containing \em text for those indexes.
		 */
		virtual QMap<int, QList<PageRelativeRectBase>> GetTextPositions (const QString& text, Qt::CaseSensitivity cs) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::ISearchableDocument,
		"org.LeechCraft.Monocle.ISearchableDocument/1.0")
