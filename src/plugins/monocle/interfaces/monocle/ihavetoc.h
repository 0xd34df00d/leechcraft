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

namespace LC::Monocle
{
	template<typename T>
	struct TOCEntryT;

	/** @brief A list of table of contents entries.
	 */
	template<typename T>
	using TOCEntryLevelT = QVector<TOCEntryT<T>>;

	/** @brief A single table of contents entry.
	 */
	template<typename T>
	struct TOCEntryT
	{
		/** @brief The navigation action corresponding to this entry.
		 */
		T Navigation_;

		/** @brief The human-readable name of the entry.
		 */
		QString Name_;

		/** @brief Child items of this entry.
		 */
		TOCEntryLevelT<T> ChildLevel_;
	};

	using TOCEntryLevel_t = TOCEntryLevelT<NavigationAction>;
	using TOCEntry = TOCEntryT<NavigationAction>;

	using TOCEntryIDLevel = TOCEntryLevelT<QByteArray>;
	using TOCEntryID = TOCEntryT<QByteArray>;

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
		virtual ~IHaveTOC () = default;

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

Q_DECLARE_INTERFACE (LC::Monocle::IHaveTOC,
		"org.LeechCraft.Monocle.IHaveTOC/1.0")
