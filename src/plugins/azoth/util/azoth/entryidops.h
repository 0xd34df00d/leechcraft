/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QByteArray>
#include <QString>
#include "azothutilconfig.h"
#include <interfaces/azoth/azothcommon.h>

namespace LC::Azoth
{
	template<typename>
	constexpr bool IsEntryIdType = false;

	template<IdKind K>
	constexpr bool IsEntryIdType<EntryId<K>> = true;

	template<IdKind K>
	constexpr bool IsEntryIdType<GlobalId<K>> = true;

	template<template<IdKind> typename Id>
	constexpr bool IsEntryIdType<StrongestId<Id>> = true;

	template<typename T>
	concept AnyEntryId = IsEntryIdType<T>;

	/** @brief Persistable, versioned, binary form of entry IDs.
	 *
	 * Unlike `ToString()`, it survives a round trip: the kind of a
	 * `StrongestId` and the account/entry boundary of a `GlobalId` are kept.
	 *
	 * Unlike `SerializeString()`, the resulting value is not necessarily
	 * null-terminated while possibly containing `NULL`s within the string,
	 * so you might want to avoid using it as a C string with C APIs.
	 *
	 * @sa Deserialize()
	 * @sa SerializeString()
	 */
	template<AnyEntryId Id>
	QByteArray SerializeBytes (const Id&);

	/** @brief Persistable, versioned, string-based form of entry IDs.
	 *
	 * Unlike `ToString()`, it survives a round trip: the kind of a
	 * `StrongestId` and the account/entry boundary of a `GlobalId` are kept.
	 *
	 * Unlike `SerializeBytes()`, the resulting value is a proper string
	 * and can be used with APIs that expect a string.
	 * On the other hand, this form is slower.
	 *
	 * @sa Deserialize()
	 * @sa SerializeBytes()
	 */
	template<AnyEntryId Id>
	QString SerializeString (const Id&);

	/** @brief Deserializes persistable binary form of entry IDs.
	 *
	 * @sa SerializeBytes()
	 * @sa SerializeString()
	 */
	template<AnyEntryId Id>
	std::optional<Id> Deserialize (const QByteArray&);

	/** @brief Deserializes persistable string form of entry IDs.
	 *
	 * @sa SerializeBytes()
	 * @sa SerializeString()
	 */
	template<AnyEntryId Id>
	std::optional<Id> Deserialize (QStringView);

	template<AnyEntryId Id>
	std::optional<Id> Deserialize (const char*) = delete ("a C string has no encoding: wrap it in QString::fromUtf8 () or QByteArray first");

	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryPersistentId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryConventionalId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalPersistentId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalConventionalId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryStrongestId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalStrongestId&);

	extern template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (const QByteArray&);
	extern template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (const QByteArray&);
	extern template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (const QByteArray&);
	extern template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (const QByteArray&);
	extern template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (const QByteArray&);
	extern template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (const QByteArray&);

	extern template AZOTH_UTIL_API QString SerializeString (const EntryPersistentId&);
	extern template AZOTH_UTIL_API QString SerializeString (const EntryConventionalId&);
	extern template AZOTH_UTIL_API QString SerializeString (const GlobalPersistentId&);
	extern template AZOTH_UTIL_API QString SerializeString (const GlobalConventionalId&);
	extern template AZOTH_UTIL_API QString SerializeString (const EntryStrongestId&);
	extern template AZOTH_UTIL_API QString SerializeString (const GlobalStrongestId&);

	extern template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (QStringView);
	extern template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (QStringView);
	extern template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (QStringView);
	extern template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (QStringView);
	extern template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (QStringView);
	extern template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (QStringView);
}
