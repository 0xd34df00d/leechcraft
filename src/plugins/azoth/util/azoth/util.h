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
class QDataStream;

namespace LC::Azoth
{
	class IAccount;
	class IMUCEntry;

	AZOTH_UTIL_API bool IsOnline (State);
	AZOTH_UTIL_API QString StateToString (State);

	AZOTH_UTIL_API void RejoinMuc (const IMUCEntry& entry);
	AZOTH_UTIL_API void RejoinMuc (IAccount& acc, const QVariantMap& identiyfingData);

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

	template<IdKind K>
	QDataStream& operator<< (QDataStream&, const EntryId<K>&);

	template<IdKind K>
	QDataStream& operator>> (QDataStream&, EntryId<K>&);

	template<IdKind K>
	QDataStream& operator<< (QDataStream&, const GlobalId<K>&);

	template<IdKind K>
	QDataStream& operator>> (QDataStream&, GlobalId<K>&);

	template<template<IdKind> typename Id>
	QDataStream& operator<< (QDataStream&, const StrongestId<Id>&);

	template<template<IdKind> typename Id>
	QDataStream& operator>> (QDataStream&, StrongestId<Id>&);

	extern template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const EntryId<IdKind::Persistent>&);
	extern template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const EntryId<IdKind::Conventional>&);
	extern template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, EntryId<IdKind::Persistent>&);
	extern template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, EntryId<IdKind::Conventional>&);

	extern template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const GlobalId<IdKind::Persistent>&);
	extern template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const GlobalId<IdKind::Conventional>&);
	extern template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, GlobalId<IdKind::Persistent>&);
	extern template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, GlobalId<IdKind::Conventional>&);

	extern template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const StrongestId<EntryId>&);
	extern template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const StrongestId<GlobalId>&);
	extern template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, StrongestId<EntryId>&);
	extern template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, StrongestId<GlobalId>&);
}
