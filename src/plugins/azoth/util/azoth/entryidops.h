/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <concepts>
#include <optional>
#include <source_location>
#include <QByteArray>
#include <QString>
#include <interfaces/azoth/azothcommon.h>
#include "azothutilconfig.h"

class QDataStream;
class QVariant;

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

	template<typename T>
	concept AnyId = AnyEntryId<T> || std::same_as<T, AccountId>;

	/** @brief Persistable, versioned, binary form of entry IDs.
	 *
	 * Unlike `ToString()`, it survives a round trip: the kind of a
	 * `StrongestId` and the account/entry boundary of a `GlobalId` are kept.
	 *
	 * Unlike `SerializeString()`, the resulting value is not necessarily
	 * null-terminated while possibly containing `NULL`s within the string,
	 * so you might want to avoid using it as a C string with C APIs.
	 *
	 * The result is exactly what `Serialize()` writes into a stream.
	 *
	 * @sa Deserialize()
	 * @sa Serialize()
	 * @sa SerializeString()
	 */
	template<AnyEntryId Id>
	QByteArray SerializeBytes (const Id&);

	/** @brief Persistable, string-based form of entry IDs.
	 *
	 * Unlike `ToString()`, it survives a round trip: the kind of a
	 * `StrongestId` and the account/entry boundary of a `GlobalId` are kept.
	 *
	 * Unlike `SerializeBytes()`, the resulting value is a proper string
	 * and can be used with APIs that expect a string.
	 * On the other hand, this form is slower.
	 *
	 * Unlike the binary form, this one is not versioned: its grammar is
	 * stable, as the resulting strings serve as persistent keys in stores
	 * outside of Azoth's control.
	 *
	 * @sa Deserialize()
	 * @sa SerializeBytes()
	 */
	template<AnyEntryId Id>
	QString SerializeString (const Id&);

	/** @brief Deserializes persistable binary form of entry IDs.
	 *
	 * The whole of the passed data must be the ID: trailing bytes make
	 * it invalid. An ID embedded into a larger record is read with the
	 * `QDataStream` overload instead.
	 *
	 * @sa SerializeBytes()
	 * @sa SerializeString()
	 */
	template<AnyEntryId Id>
	std::optional<Id> Deserialize (const QByteArray&, std::source_location = std::source_location::current ());

	/** @brief Deserializes persistable string form of entry IDs.
	 *
	 * @sa SerializeBytes()
	 * @sa SerializeString()
	 */
	template<AnyEntryId Id>
	std::optional<Id> Deserialize (QStringView, std::source_location = std::source_location::current ());

	template<AnyEntryId Id>
	std::optional<Id> Deserialize (const char*) = delete ("a C string has no encoding: wrap it in QString::fromUtf8 () or QByteArray first");

	/** @brief Writes the binary form of an entry ID into a stream.
	 *
	 * This allows embedding an entry ID into a larger stream-based record.
	 * The written bytes are exactly those `SerializeBytes()` returns.
	 *
	 * @sa Deserialize()
	 * @sa SerializeBytes()
	 */
	template<AnyEntryId Id>
	void Serialize (QDataStream&, const Id&);

	/** @brief Reads an entry ID written by `Serialize()` from a stream.
	 *
	 * On success, exactly the bytes written by `Serialize()` are consumed.
	 * On failure, the stream status is set accordingly, and the stream
	 * position is unspecified.
	 *
	 * @sa Serialize()
	 */
	template<AnyEntryId Id>
	std::optional<Id> Deserialize (QDataStream&, std::source_location = std::source_location::current ());

	template<AnyId Id>
	std::optional<Id> IdFromVariant (const QVariant&);

	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryPersistentId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryConventionalId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalPersistentId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalConventionalId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryStrongestId&);
	extern template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalStrongestId&);

	extern template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (const QByteArray&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (const QByteArray&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (const QByteArray&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (const QByteArray&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (const QByteArray&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (const QByteArray&, std::source_location);

	extern template AZOTH_UTIL_API QString SerializeString (const EntryPersistentId&);
	extern template AZOTH_UTIL_API QString SerializeString (const EntryConventionalId&);
	extern template AZOTH_UTIL_API QString SerializeString (const GlobalPersistentId&);
	extern template AZOTH_UTIL_API QString SerializeString (const GlobalConventionalId&);
	extern template AZOTH_UTIL_API QString SerializeString (const EntryStrongestId&);
	extern template AZOTH_UTIL_API QString SerializeString (const GlobalStrongestId&);

	extern template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (QStringView, std::source_location);
	extern template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (QStringView, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (QStringView, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (QStringView, std::source_location);
	extern template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (QStringView, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (QStringView, std::source_location);

	extern template AZOTH_UTIL_API void Serialize (QDataStream&, const EntryPersistentId&);
	extern template AZOTH_UTIL_API void Serialize (QDataStream&, const EntryConventionalId&);
	extern template AZOTH_UTIL_API void Serialize (QDataStream&, const GlobalPersistentId&);
	extern template AZOTH_UTIL_API void Serialize (QDataStream&, const GlobalConventionalId&);
	extern template AZOTH_UTIL_API void Serialize (QDataStream&, const EntryStrongestId&);
	extern template AZOTH_UTIL_API void Serialize (QDataStream&, const GlobalStrongestId&);

	extern template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (QDataStream&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (QDataStream&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (QDataStream&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (QDataStream&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (QDataStream&, std::source_location);
	extern template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (QDataStream&, std::source_location);

	extern template AZOTH_UTIL_API std::optional<AccountId> IdFromVariant<AccountId> (const QVariant&);
	extern template AZOTH_UTIL_API std::optional<EntryPersistentId> IdFromVariant<EntryPersistentId> (const QVariant&);
	extern template AZOTH_UTIL_API std::optional<EntryConventionalId> IdFromVariant<EntryConventionalId> (const QVariant&);
	extern template AZOTH_UTIL_API std::optional<GlobalPersistentId> IdFromVariant<GlobalPersistentId> (const QVariant&);
	extern template AZOTH_UTIL_API std::optional<GlobalConventionalId> IdFromVariant<GlobalConventionalId> (const QVariant&);
	extern template AZOTH_UTIL_API std::optional<EntryStrongestId> IdFromVariant<EntryStrongestId> (const QVariant&);
	extern template AZOTH_UTIL_API std::optional<GlobalStrongestId> IdFromVariant<GlobalStrongestId> (const QVariant&);
}
