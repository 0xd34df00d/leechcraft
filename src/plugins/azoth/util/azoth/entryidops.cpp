/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entryidops.h"
#include <type_traits>
#include <utility>
#include <QDataStream>
#include <QtDebug>
#include <util/sll/logging.h>
#include <util/sys/serialization.h>
#include "util.h"

namespace LC::Azoth
{
	namespace
	{
		constexpr quint8 BytesVersion = 1;

		constexpr char KindChar (IdKind kind)
		{
			switch (kind)
			{
			case IdKind::Persistent:
				return 'P';
			case IdKind::Conventional:
				return 'C';
			}
			std::unreachable ();
		}

		std::optional<IdKind> KindFromChar (QChar ch)
		{
			switch (ch.toLatin1 ())
			{
			case KindChar (IdKind::Persistent):
				return IdKind::Persistent;
			case KindChar (IdKind::Conventional):
				return IdKind::Conventional;
			default:
				return {};
			}
		}

		template<IdKind K>
		QString ToSerializedString (const EntryId<K>& id)
		{
			return id.ToString ();
		}

		template<IdKind K>
		QString ToSerializedString (const GlobalId<K>& id)
		{
			return QString::number (id.AccountId_.size ()) + ':' + id.AccountId_ + ':' + ToSerializedString (id.EntryId_);
		}

		template<template<IdKind> typename Id>
		QString ToSerializedString (const StrongestId<Id>& id)
		{
			return std::visit ([]<IdKind K> (const Id<K>& concrete)
					{
						return QString { QLatin1Char { KindChar (K) } } + ':' + ToSerializedString (concrete);
					},
					id.ToVariant ());
		}

		template<IdKind K>
		std::optional<EntryId<K>> FromSerializedString (std::type_identity<EntryId<K>>, QStringView str)
		{
			return EntryId<K>::FromString (str.toString ());
		}

		template<IdKind K>
		std::optional<GlobalId<K>> FromSerializedString (std::type_identity<GlobalId<K>>, QStringView str)
		{
			const auto lenEnd = str.indexOf (':');
			if (lenEnd < 0)
				return {};

			bool ok = false;
			const auto accLen = str.first (lenEnd).toInt (&ok);
			if (!ok || accLen < 0)
				return {};

			const auto accStart = lenEnd + 1;
			const auto accEnd = accStart + accLen;
			if (accEnd >= str.size () || str [accEnd] != ':')
				return {};

			const auto entryId = FromSerializedString (std::type_identity<EntryId<K>> {}, str.sliced (accEnd + 1));
			if (!entryId)
				return {};

			return GlobalId<K> { str.sliced (accStart, accLen).toString (), *entryId };
		}

		template<template<IdKind> typename Id>
		std::optional<StrongestId<Id>> FromSerializedString (std::type_identity<StrongestId<Id>>, QStringView str)
		{
			if (str.size () < 2 || str [1] != ':')
				return {};

			const auto kind = KindFromChar (str [0]);
			if (!kind)
				return {};

			const auto rest = str.sliced (2);
			switch (*kind)
			{
			case IdKind::Persistent:
				return FromSerializedString (std::type_identity<Id<IdKind::Persistent>> {}, rest);
			case IdKind::Conventional:
				return FromSerializedString (std::type_identity<Id<IdKind::Conventional>> {}, rest);
			}
			std::unreachable ();
		}
	}

	template<AnyEntryId Id>
	QByteArray SerializeBytes (const Id& id)
	{
		QByteArray result;
		QDataStream stream { &result, QDataStream::WriteOnly };
		stream << BytesVersion << id;
		return result;
	}

	template<AnyEntryId Id>
	std::optional<Id> Deserialize (const QByteArray& data, std::source_location loc)
	{
		QDataStream stream { data };
		if (!Util::EnsureVersion<quint8> (stream, 1, BytesVersion, loc))
			return {};

		Id id;
		stream >> id;
		if (stream.status () != QDataStream::Ok)
		{
			Util::LogAt (loc).warning () << "corrupt entry id:" << stream.status ();
			return {};
		}

		return id;
	}

	template<AnyEntryId Id>
	QString SerializeString (const Id& id)
	{
		return ToSerializedString (id);
	}

	template<AnyEntryId Id>
	std::optional<Id> Deserialize (QStringView str, std::source_location loc)
	{
		auto result = FromSerializedString (std::type_identity<Id> {}, str);
		if (!result)
			Util::LogAt (loc).warning () << "unparseable entry id:" << str;
		return result;
	}

	template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryPersistentId&);
	template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryConventionalId&);
	template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalPersistentId&);
	template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalConventionalId&);
	template AZOTH_UTIL_API QByteArray SerializeBytes (const EntryStrongestId&);
	template AZOTH_UTIL_API QByteArray SerializeBytes (const GlobalStrongestId&);

	template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (const QByteArray&, std::source_location);
	template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (const QByteArray&, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (const QByteArray&, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (const QByteArray&, std::source_location);
	template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (const QByteArray&, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (const QByteArray&, std::source_location);

	template AZOTH_UTIL_API QString SerializeString (const EntryPersistentId&);
	template AZOTH_UTIL_API QString SerializeString (const EntryConventionalId&);
	template AZOTH_UTIL_API QString SerializeString (const GlobalPersistentId&);
	template AZOTH_UTIL_API QString SerializeString (const GlobalConventionalId&);
	template AZOTH_UTIL_API QString SerializeString (const EntryStrongestId&);
	template AZOTH_UTIL_API QString SerializeString (const GlobalStrongestId&);

	template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (QStringView, std::source_location);
	template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (QStringView, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (QStringView, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (QStringView, std::source_location);
	template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (QStringView, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (QStringView, std::source_location);
}
