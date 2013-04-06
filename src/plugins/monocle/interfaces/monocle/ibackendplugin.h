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

#include <QString>
#include <QMetaType>
#include "idocument.h"

namespace LeechCraft
{
namespace Monocle
{
	/** @brief Basic interface for format backends plugins for Monocle.
	 *
	 * This interface should be implemented by plugins that provide
	 * format backends for Monocle document reader — that is, for those
	 * plugins that can load documents.
	 *
	 * @sa IDocument
	 */
	class IBackendPlugin
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IBackendPlugin () {}

		/** @brief Checks whether the given document can be loaded.
		 *
		 * This method should return <code>true</code> if the document
		 * can possibly be loaded and <code>false</code> otherwise.
		 *
		 * The cheaper this function is, the better. It is discouraged to
		 * check by extension, though.
		 *
		 * It is OK to return nullptr or invalid document from
		 * LoadDocument() even if this method returns <code>true</code>
		 * for a given document.
		 *
		 * @param[filename] in Path to the document to check.
		 * @return Whether the document at \em filename can be loaded.
		 *
		 * @sa LoadDocument()
		 */
		virtual bool CanLoadDocument (const QString& filename) = 0;

		/** @brief Loads the given document.
		 *
		 * This method should load the document at \em filename and
		 * return a pointer to it, or a null pointer if the document is
		 * invalid.
		 *
		 * The ownership is passed to the caller: that is, this backend
		 * plugin should keep no strong shared references to the returned
		 * document.
		 *
		 * It is OK for this method to return a null or invalid document
		 * even if CanLoadDocument() returned <code>true</code> for this
		 * \em filename,
		 *
		 * @param[filename] in The document to load.
		 * @return The document object for \em filename, or null pointer,
		 * or invalid document if an error has occurred.
		 *
		 * @sa LoadDocument(), IDocument
		 */
		virtual IDocument_ptr LoadDocument (const QString& filename) = 0;

		/** @brief Returns true whether the backend is threaded.
		 *
		 * This function returns true if the implementation supports
		 * threaded pages rendering.
		 *
		 * The default implementation simply returns false.
		 *
		 * @return Whether threaded rendering is supported by this
		 * backend.
		 */
		virtual bool IsThreaded () const
		{
			return false;
		}
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::IBackendPlugin,
		"org.LeechCraft.Monocle.IBackendPlugin/1.0");
