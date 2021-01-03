/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "xdgconfig.h"

class QString;
class QIcon;
class QPixmap;

namespace LC::Util::XDG
{
	UTIL_XDG_API QIcon GetAppIcon (const QString& iconName);
	UTIL_XDG_API QPixmap GetAppPixmap (const QString& iconName);
}
