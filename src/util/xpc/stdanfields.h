/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "xpcconfig.h"

template<typename T>
class QList;
class QString;

namespace LC::AN
{
	struct FieldData;
}

namespace LC::Util
{
	/** @brief Returns the list of the standard AN fields for the given
	 * \em category.
	 *
	 * @note Plugins may define their own fields for different
	 * categories. The IANEmitter interface is used by those plugins to
	 * communicate their custom fields to the rest of LeechCraft.
	 *
	 * @param[in] category The category of events to return fields for.
	 * @return The descriptions of the standard fields used by events in
	 * the given category.
	 *
	 * @sa LC::AN::FieldData
	 */
	UTIL_XPC_API QList<LC::AN::FieldData> GetStdANFields (const QString& category);
}
