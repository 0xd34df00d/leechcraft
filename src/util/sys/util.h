/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sysconfig.h"

class QString;

namespace LC
{
namespace Util
{
	/** @brief Checks if the given filename has a known image extension.
	 *
	 * This function checks if the \em filename has an extension matching
	 * any image format known to QImageWriter.
	 *
	 * @param[in] filename The name of the file, either relative or absolute.
	 * @return Whether the \em filename has a known image suffix.
	 */
	UTIL_SYS_API bool HasSupportedImageExtension (const QString& filename);

	UTIL_SYS_API bool IsOSXLoadFromBundle ();
}
}
