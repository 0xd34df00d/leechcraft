/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QDateTime>
#include <QFontMetrics>
#include <QLocale>

namespace LC::Aggregator
{
	int GetDateColumnWidth (const QFontMetrics& fm)
	{
		return fm.horizontalAdvance (QLocale {}.toString (QDateTime::currentDateTime (), QLocale::ShortFormat) + "__");
	}
}
