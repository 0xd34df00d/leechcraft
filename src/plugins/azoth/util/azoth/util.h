/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "azothutilconfig.h"
#include <interfaces/azoth/azothcommon.h>

namespace LC::Azoth
{
	class IAccount;
	class IMUCEntry;

	AZOTH_UTIL_API bool IsOnline (State);
	AZOTH_UTIL_API QString StateToString (State);

	AZOTH_UTIL_API void RejoinMuc (const IMUCEntry& entry);
	AZOTH_UTIL_API void RejoinMuc (IAccount& acc, const QVariantMap& identiyfingData);
}
