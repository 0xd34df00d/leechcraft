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

#include <QMetaType>
#include "ilink.h"

namespace LeechCraft
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

Q_DECLARE_INTERFACE (LeechCraft::Monocle::IHaveTOC,
		"org.LeechCraft.Monocle.IHaveTOC/1.0");
