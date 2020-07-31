/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

namespace LC
{
namespace Monocle
{
	class IAnnotation;
	typedef std::shared_ptr<IAnnotation> IAnnotation_ptr;

	/** @brief Interface for documents supporting annotations.
	 *
	 * If the document format can contain annotations, this interface
	 * should be implemented.
	 *
	 * @sa IAnnotation
	 */
	class ISupportAnnotations
	{
	public:
		virtual ~ISupportAnnotations () {}

		/** @brief Returns the list of annotations on the given page.
		 *
		 * @param[in] page The page to query.
		 * @return The list of annotations on the \em page.
		 */
		virtual QList<IAnnotation_ptr> GetAnnotations (int page) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::ISupportAnnotations,
		"org.LeechCraft.Monocle.ISupportAnnotations/1.0")
