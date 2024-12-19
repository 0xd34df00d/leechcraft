/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include "xdgconfig.h"

namespace LC::Util::XDG
{
	/** @brief Describes the various types of XDG <code>.desktop</code>
	 * files.
	 */
	enum class Type
	{
		/** @brief Unknown type.
		 */
		Other,

		/** @brief A shortcut to an application.
		 */
		Application,

		/** @brief A shortcut to an URL.
		 */
		URL,

		/** @brief A shortcut to a directory.
		 */
		Dir
	};

	/** @brief Returns a set of typical directories with
	 * <code>desktop</code> files of the given \em types.
	 *
	 * @param[in] types The types of the interesting <code>.desktop</code>
	 * files.
	 * @return The list of the directories where the said files typically
	 * reside.
	 */
	UTIL_XDG_API QStringList ToPaths (const QList<Type>& types);
}
