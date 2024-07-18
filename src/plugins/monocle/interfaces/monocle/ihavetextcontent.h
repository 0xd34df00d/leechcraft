/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QString;

namespace LC
{
namespace Monocle
{
	/** @brief Interface for documents supporting querying text contents.
	 *
	 * This interface should be implemented by the documents of formats
	 * supporting obtaining the text contained in some page rectangle.
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
		 * If \em rect is empty or null, the text from the whole page
		 * should be returned.
		 *
		 * @param[in] page The index of the page to query.
		 * @param[in] rect The rectangle on the \em page to query.
		 * @return The text in \em rect at \em page.
		 */
		virtual QString GetTextContent (int page, const PageRelativeRectBase& rect) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IHaveTextContent,
		"org.LeechCraft.Monocle.IHaveTextContent/1.0")
