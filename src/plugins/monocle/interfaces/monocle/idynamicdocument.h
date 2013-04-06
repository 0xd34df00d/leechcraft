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

namespace LeechCraft
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
	 * IDynamicDocument *idd;
	 * connect (dynamic_cast<IDocument*> (idd)->GetQObject (),
	 *         SIGNAL (pageSizeChanged (int)),
	 *         this,
	 *         SLOT (handlePageSizeChanged (int)));
	 * \endcode
	 */
	class IDynamicDocument
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IDynamicDocument () {}
	protected:
		/** @brief Emitted when the size of the given page is changed.
		 *
		 * The signal is emitted after the new size is known, so
		 * <code>IDocument::GetPageSize(page)</code> should already
		 * return the new value.
		 *
		 * @param[out] page The index of the page that has been changed.
		 */
		virtual void pageSizeChanged (int page) = 0;

		/** @brief Emitted when contents of the given page are changed.
		 *
		 * The \em page should typically be re-rendered after this signal.
		 *
		 * @param[out] page The index of the page that has been changed.
		 */
		virtual void pageContentsChanged (int page) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::IDynamicDocument,
		"org.LeechCraft.Monocle.IDynamicDocument/1.0");
