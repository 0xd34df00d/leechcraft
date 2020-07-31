/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace Monocle
{
	/** @brief Implemented by documents whose pages can change dynamically.
	 *
	 * This interface should be implemented by documents whose pages can
	 * change "themselves", including only page sizes and page contents.
	 * Changes in forms, annotations and links (if any) should be
	 * propagated via the corresponding classes.
	 *
	 * The primary use of this interface is for format backends that load
	 * the documents asynchronously. An example of such backend is the
	 * DjVuLibre-based Seen plugin.
	 *
	 * This class has some signals, and one can use the
	 * IDocument::GetQObject() method to get an object of this class as a
	 * QObject and connect to those signals:
	 * \code
		IDynamicDocument *idd = ...;
		connect (dynamic_cast<IDocument*> (idd)->GetQObject (),
				SIGNAL (pageSizeChanged (int)),
				this,
				SLOT (handlePageSizeChanged (int)));
	   \endcode
	 */
	class IDynamicDocument
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IDynamicDocument () {}
	protected:
		/** @brief Emitted when the size of the given \em page is changed.
		 *
		 * The signal is emitted after the new size is known, so
		 * <code>IDocument::GetPageSize(page)</code> should already
		 * return the new value.
		 *
		 * @param[out] page The index of the page that has been changed.
		 */
		virtual void pageSizeChanged (int page) = 0;

		/** @brief Emitted when contents of the given \em page are changed.
		 *
		 * The \em page should typically be re-rendered after this signal.
		 *
		 * @param[out] page The index of the page that has been changed.
		 */
		virtual void pageContentsChanged (int page) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IDynamicDocument,
		"org.LeechCraft.Monocle.IDynamicDocument/1.0")
