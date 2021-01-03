/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QStringList>
#include "xdgconfig.h"

namespace LC::Util::XDG
{
	/** @brief A parser for XDG <code>.desktop</code> files.
	 *
	 * This parser does not produce any structured information. Instead,
	 * it only returns a hash from group name to corresponding group
	 * fields (see Result_t). A more structured representation is
	 * provided by the Item class.
	 *
	 * @sa Item
	 */
	class DesktopParser
	{
	public:
		/** @brief Mapping from a language to the list of values for that
		 * language.
		 *
		 * "No language" corresponds to a null string.
		 */
		using LangValue_t = QHash<QString, QStringList>;

		/** @brief Mapping from a field name to the list of
		 * language-dependent values of that field.
		 */
		using Group_t = QHash<QString, LangValue_t>;

		/** @brief Mapping from a group name to the group itself.
		 */
		using Result_t = QHash<QString, Group_t>;

		/** @brief Parses the XDG \em data.
		 *
		 * @param[in] data The byte array containing XDG
		 * <code>.desktop</code> file data.
		 * @return The set of groups in the XDG data.
		 */
		UTIL_XDG_API Result_t operator() (const QByteArray& data);
	};
}
