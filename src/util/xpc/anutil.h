/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/an/ianemitter.h>
#include "xpcconfig.h"

namespace LC::Util::AN
{
	/** @brief Returns the map from the category ID to its name.
	 *
	 * @returns The map from the category ID to its name.
	 */
	UTIL_XPC_API QMap<QString, QString> GetCategoryNameMap ();

	/** @brief Returns the known events types for the given \em category.
	 *
	 * This function returns the list of known standard event types for
	 * the \em category, or an empty list if the \em category is not
	 * known.
	 *
	 * @return The list of events for the \em category.
	 */
	UTIL_XPC_API QStringList GetKnownEventTypes (const QString& category);

	/** @brief Returns the human-readable name of the event \em category.
	 *
	 * If the \em category is not known, this function just returns the
	 * \em category string.
	 *
	 * @param[in] category The ID of the event category.
	 * @return The human-readable name of the category, or \em category
	 * if it is not known.
	 *
	 * @sa GetANTypeName()
	 * @sa GetCategoryNameMap()
	 */
	UTIL_XPC_API QString GetCategoryName (const QString& category);

	/** @brief Returns the human-readable name of the event \em type.
	 *
	 * If the \em type is not known, this function just returns the
	 * \em type string.
	 *
	 * @param[in] type The ID of the event type.
	 * @return The human-readable name of the type, or \em type if it is
	 * not known.
	 *
	 * @sa GetANCategoryName()
	 */
	UTIL_XPC_API QString GetTypeName (const QString& type);

	UTIL_XPC_API QVariant ToVariant (const LC::AN::StringValueMatcher::Pattern& matcher);
	UTIL_XPC_API std::optional<LC::AN::StringValueMatcher::Pattern> StringPatternFromVariant (const QVariant& variant);

	[[deprecated]]
	UTIL_XPC_API bool Matches (const QString& string, const LC::AN::StringValueMatcher::Pattern& matcher);
	[[deprecated]]
	UTIL_XPC_API bool Matches (const QStringList& strings, const LC::AN::StringValueMatcher::Pattern& matcher);

	UTIL_XPC_API bool Matches (const QString& string, const LC::AN::StringValueMatcher& matcher);
	UTIL_XPC_API bool Matches (const QStringList& strings, const LC::AN::StringValueMatcher& matcher);
	UTIL_XPC_API bool Matches (const QUrl& url, const LC::AN::StringValueMatcher& matcher);
	UTIL_XPC_API bool Matches (bool value, const LC::AN::BoolValueMatcher& matcher);
	UTIL_XPC_API bool Matches (int value, const LC::AN::IntValueMatcher& matcher);

	UTIL_XPC_API bool Matches (const QVariant& var, QMetaType::Type expected, const LC::AN::ValueMatcher& matcher);
}
