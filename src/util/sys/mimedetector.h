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

namespace LC::Util
{
	struct MimeDetector
	{
		UTIL_SYS_API QString Detect (const QString&);
		UTIL_SYS_API QString operator() (const QString&);
	};

	UTIL_SYS_API QString DetectFileMime (const QString&);
}
