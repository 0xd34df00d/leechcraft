/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QAbstractItemModel;

namespace LC
{
namespace Monocle
{
	/** @brief Interface for documents that can toggle showing some of
	 * their contents.
	 *
	 * The list of togglable contents is returned via the
	 * GetOptContentModel(), returning a model whose items can be checked
	 * or unchecked (and, possibly, edited).
	 */
	class IHaveOptionalContent
	{
	public:
		virtual ~IHaveOptionalContent () {}

		/** @brief Returns the optional contents model for the document.
		 *
		 * @return The contents model, or a nullptr if no optional
		 * content is present in this document.
		 */
		virtual QAbstractItemModel* GetOptContentModel () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IHaveOptionalContent,
		"org.LeechCraft.Monocle.IHaveOptionalContent/1.0")
