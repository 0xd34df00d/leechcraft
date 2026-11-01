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

class QDebug;

namespace LC::Azoth
{
	class IAccount;
	class ICLEntry;
	class IMUCEntry;

	AZOTH_UTIL_API bool IsOnline (State);
	AZOTH_UTIL_API QString StateToString (State);
	AZOTH_UTIL_API QString StateToID (State);

	AZOTH_UTIL_API QString FormatNameWithAddress (const ICLEntry&);

	AZOTH_UTIL_API void RejoinMuc (const IMUCEntry& entry);
	AZOTH_UTIL_API void RejoinMuc (IAccount& acc, const QVariantMap& identiyfingData);

	AZOTH_UTIL_API QDebug operator<< (QDebug, const AccountId&);

	template<IdKind K>
	QDebug operator<< (QDebug, const EntryId<K>&);

	template<IdKind K>
	QDebug operator<< (QDebug, const GlobalId<K>&);

	template<template<IdKind> typename Id>
	QDebug operator<< (QDebug, const StrongestId<Id>&);

	extern template AZOTH_UTIL_API QDebug operator<< (QDebug, const EntryId<IdKind::Persistent>&);
	extern template AZOTH_UTIL_API QDebug operator<< (QDebug, const EntryId<IdKind::Conventional>&);

	extern template AZOTH_UTIL_API QDebug operator<< (QDebug, const GlobalId<IdKind::Persistent>&);
	extern template AZOTH_UTIL_API QDebug operator<< (QDebug, const GlobalId<IdKind::Conventional>&);

	extern template AZOTH_UTIL_API QDebug operator<< (QDebug, const StrongestId<EntryId>&);
	extern template AZOTH_UTIL_API QDebug operator<< (QDebug, const StrongestId<GlobalId>&);
}
