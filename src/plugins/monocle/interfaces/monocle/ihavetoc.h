/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaType>
#include "ilink.h"

namespace LC
{
namespace Monocle
{
	struct TOCEntry;

	/** @brief A list of table of contents entries.
	 */
	typedef QList<TOCEntry> TOCEntryLevel_t;

	/** @brief A single table of contents entry.
	 */
	struct TOCEntry
	{
		/** @brief The link action corresponding to this entry.
		 *
		 * This link should be executed when the entry is activated.
		 *
		 * @sa ILink
		 */
		ILink_ptr Link_;

		/** @brief The human-readable name of the entry.
		 */
		QString Name_;

		/** @brief Child items of this entry.
		 */
		TOCEntryLevel_t ChildLevel_;
	};

	/** @brief Interface for documents supporting table of contents.
	 *
	 * This interface should be implemented by the documents of formats
	 * supporting having table of contents.
	 */
	class IHaveTOC
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IHaveTOC () {}

		/** @brief Returns the root level of the TOC.
		 *
		 * If the root level is empty, there is no table of contents for
		 * this document.
		 *
		 * @return Returns the root level of the TOC.
		 */
		virtual TOCEntryLevel_t GetTOC () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IHaveTOC,
		"org.LeechCraft.Monocle.IHaveTOC/1.0")
