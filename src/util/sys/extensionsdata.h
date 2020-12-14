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
class QIcon;

namespace LC::Util
{
	class ExtensionsData
	{
		ExtensionsData () = default;

		ExtensionsData (const ExtensionsData&) = delete;
		ExtensionsData& operator= (const ExtensionsData&) = delete;
	public:
		UTIL_SYS_API static ExtensionsData& Instance ();

		UTIL_SYS_API QString GetMime (const QString& extension) const;
		UTIL_SYS_API QIcon GetExtIcon (const QString& extension) const;
		UTIL_SYS_API QIcon GetMimeIcon (const QString& mime) const;
	};
}
