/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "azothutilconfig.h"

namespace LC::Azoth::Fields
{
	AZOTH_UTIL_API extern const QString Msg;
	AZOTH_UTIL_API extern const QString NewStatus;
	AZOTH_UTIL_API extern const QString SourceName;
	AZOTH_UTIL_API extern const QString SourceAccountID;
	AZOTH_UTIL_API extern const QString SourceHumanReadableAddress;
	AZOTH_UTIL_API extern const QString SourceGroups;
	AZOTH_UTIL_API extern const QString RoomName;
	AZOTH_UTIL_API extern const QString RoomHumanReadableAddress;
}

namespace LC::Azoth::Fields::Internal
{
	AZOTH_UTIL_API extern const QString SourceGlobalStrongestID;
	AZOTH_UTIL_API extern const QString RoomGlobalStrongestID;
}
