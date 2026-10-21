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

	/** @brief Returns the delimiter the tags are split by for the given
	 * separator they are joined with.
	 *
	 * The delimiter is the separator without the whitespace around it, or
	 * the separator itself if there is nothing else to it.
	 *
	 * @param[in] separator The separator of the tags.
	 * @return The delimiter of the tags.
	 *
	 * @sa GetDefaultTagsSeparator()
	 * @sa SplitTags()
	 */
	UTIL_TAGS_API QString GetTagsDelimiter (const QString& separator);

	/** @brief Splits a string with tags into the tags.
	 *
	 * The tags are separated by the delimiter corresponding to the
	 * `separator`, see `GetTagsDelimiter()`. The tags are trimmed, and the
	 * empty ones are dropped.
	 *
	 * @param[in] text The string with the tags.
	 * @param[in] separator The separator of the tags.
	 * @return The tags.
	 */
	UTIL_TAGS_API QStringList SplitTags (QStringView text, const QString& separator);
}
