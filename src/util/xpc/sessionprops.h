/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "xpcconfig.h"
#include <interfaces/ihaverecoverabletabs.h>

class QObject;

namespace LC::Util
{
	/** @brief Checks whether a tab session keeps the dynamic property `name`.
	 *
	 * Tab sessions keep the properties whose names start with `SessionData/`.
	 *
	 * @sa GetSessionProps()
	 */
	UTIL_XPC_API bool IsSessionProp (const QByteArray& name);

	/** @brief Returns the dynamic properties of `tab` that a tab session keeps.
	 *
	 * @sa IsSessionProp(), SetSessionProps()
	 */
	UTIL_XPC_API DynPropertiesList_t GetSessionProps (const QObject& tab);

	/** @brief Sets the session properties among `props` as dynamic properties of `tab`.
	 *
	 * Entries rejected by `IsSessionProp()` are skipped.
	 *
	 * @sa GetSessionProps()
	 */
	UTIL_XPC_API void SetSessionProps (QObject& tab, const DynPropertiesList_t& props);
}
