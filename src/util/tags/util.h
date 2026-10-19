/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include "tagsconfig.h"

namespace LC::Util
{
	UTIL_TAGS_API QString GetDefaultTagsSeparator ();

	/** @brief Splits a string with tags into the tags.
	 *
	 * The whitespace around the separator and around the tags is dropped,
	 * and so are the empty tags.
	 *
	 * @param[in] text The string with the tags.
	 * @param[in] separator The separator of the tags.
	 * @return The tags.
	 *
	 * @sa GetDefaultTagsSeparator()
	 */
	UTIL_TAGS_API QStringList SplitTags (const QString& text, const QString& separator);
}
