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

#include <QtPlugin>

class QString;
class QRect;

namespace LeechCraft
{
namespace Monocle
{
	/** @brief Interface for documents supporting querying text contents.
	 *
	 * This interface should be implemented by the documents of formats
	 * supporting obtaining the text contained in a selection rectangle.
	 */
	class IHaveTextContent
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IHaveTextContent () {}

		/** @brief Returns the text in the given rectangle.
		 *
		 * This function should return the text contained in the given
		 * \em rect at the given \em page, or an empty string if there is
		 * no text in this \em rect or the document doesn't contain any
		 * text information.
		 *
		 * @param[in] page The index of the page to query.
		 * @param[in] rect The rectangle on the \em page to query.
		 * @return The text in \em rect at \em page.
		 */
		virtual QString GetTextContent (int page, const QRect& rect) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::IHaveTextContent,
		"org.LeechCraft.Monocle.IHaveTextContent/1.0");
