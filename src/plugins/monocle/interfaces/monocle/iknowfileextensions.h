/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QtPlugin>

namespace LC
{
namespace Monocle
{
	/** @brief Interface for backend plugins providing default extensions
	 * for their relevant file types.
	 *
	 * This interface is used by Monocle core to know which file types
	 * with which extensions can be handled. It may then, for example,
	 * provide separate filters for each of the document types in a file
	 * open dialog.
	 *
	 * For example, a PDF backend plugin may know that PDF documents
	 * typically has \em .pdf extension.
	 */
	class IKnowFileExtensions
	{
	protected:
		virtual ~IKnowFileExtensions () = default;
	public:
		/** @brief Describes a single typical file type.
		 *
		 * A single file type may have multiple typical extensions in the
		 * wild, thus it contains a list of extensions in the Extensions_
		 * field.
		 */
		struct ExtInfo
		{
			/** @brief Human-readable description.
			 *
			 * For example, "DjVu files".
			 */
			QString Description_;

			/** @brief Typically used extensions for the file type.
			 *
			 * For example, for DjVu format this will have \em djvu and
			 * \em djvu extensions.
			 */
			QList<QString> Extensions_;
		};

		/** @brief Returns the list of supported typical file extensions.
		 *
		 * @return The list of ExtInfo objects.
		 */
		virtual QList<ExtInfo> GetKnownFileExtensions () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IKnowFileExtensions,
		"org.LeechCraft.Monocle.IKnowFileExtensions/1.0")
