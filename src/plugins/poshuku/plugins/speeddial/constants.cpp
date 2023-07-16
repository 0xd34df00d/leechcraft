/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "constants.h"
#include <util/sll/qtutil.h>

namespace LC::Poshuku::SpeedDial
{
	const QString SpeedDialHost = "speeddial"_qs;
	const QString SpeedDialUrl = "lc://speeddial"_qs;

	const QString ThumbPath = "/thumb"_qs;
	const QString ThumbUrlBase = SpeedDialUrl + ThumbPath;
	const QString ThumbUrlKey = "url"_qs;

}
