/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_VERSIONCOMPARATOR_H
#define PLUGINS_LACKMAN_VERSIONCOMPARATOR_H
#include <QString>

namespace LC
{
namespace LackMan
{
	bool IsVersionLess (const QString& lver, const QString& rver);
}
}

#endif
