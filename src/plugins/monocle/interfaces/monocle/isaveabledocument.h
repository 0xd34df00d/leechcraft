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
		virtual ~ISaveableDocument () = default;

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

Q_DECLARE_INTERFACE (LC::Monocle::ISaveableDocument,
		"org.LeechCraft.Monocle.ISaveableDocument/1.0")
