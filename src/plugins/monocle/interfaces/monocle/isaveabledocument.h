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
	/** @brief Interface for documents that can be saved.
	 *
	 * This interface should be implemented by documents for formats that
	 * allow saving the document after its editable elements (like forms
	 * or annotations) were modified.
	 *
	 * Not all documents of the same format support saving: for example,
	 * encrypted PDF documents cannot be saved, while regular ones can.
	 * Thus the CanSave() method checks whether this particular document
	 * can be saved.
	 *
	 * @sa ISupportForms, ISupportAnnotations
	 */
	class ISaveableDocument
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~ISaveableDocument () {}

		/** @brief Describes the result of check for the possibility of
		 * saving.
		 */
		struct SaveQueryResult
		{
			/** @brief Whether the document can be saved.
			 */
			bool CanSave_;

			/** @brief Human-readable reason string.
			 *
			 * If the document cannot be saved, this field contains the
			 * human-readable string containing the reason why it can't
			 * be saved.
			 */
			QString Reason_;
		};

		/** @brief Checks whether this document can be saved.
		 *
		 * This method should check if the document can be saved and
		 * return a proper SaveQueryResult.
		 *
		 * @return Whether this document can be saved, and reason string
		 * if it can't.
		 */
		virtual SaveQueryResult CanSave () const = 0;

		/** @brief Saves the document at the given path.
		 *
		 * The \em path can be equal to the original document path,
		 * plugins should take this into account.
		 *
		 * @param[in] path The full path to the target file including the
		 * file name.
		 * @return Whether the document is saved successfully.
		 */
		virtual bool Save (const QString& path) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::ISaveableDocument,
		"org.LeechCraft.Monocle.ISaveableDocument/1.0");
