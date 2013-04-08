/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QRectF>
#include <QMap>
#include <QtPlugin>

namespace LeechCraft
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
		 * Rectangles should be in page coordinates, that is, with width
		 * from 0 to page's width and height from 0 to page's height.
		 * 
		 * If a page doesn't contain any occurrences of \em text it
		 * should better be omitted from the map for performance reasons.
		 * 
		 * @param[in] text The text to search for.
		 * @param[in] cs The case sensitivity of the search.
		 * @return The map from page indexes to list of rectangles
		 * containing \em text for those indexes.
		 */
		virtual QMap<int, QList<QRectF>> GetTextPositions (const QString& text, Qt::CaseSensitivity cs) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::ISearchableDocument,
		"org.LeechCraft.Monocle.ISearchableDocument/1.0");
