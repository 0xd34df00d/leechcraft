/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entryidops.h"
#include <compare>
#include <type_traits>
#include <utility>
#include <QDataStream>
#include <QIODevice>
#include <QVariant>
#include <QtDebug>
#include <util/sll/logging.h>
#include <util/sys/serialization.h>

namespace LC::Azoth
{
	static_assert (std::three_way_comparable<EntryPersistentId, std::strong_ordering>);
	static_assert (std::three_way_comparable<EntryConventionalId, std::strong_ordering>);
	static_assert (std::three_way_comparable<GlobalPersistentId, std::strong_ordering>);
	static_assert (std::three_way_comparable<GlobalConventionalId, std::strong_ordering>);
	static_assert (std::three_way_comparable<EntryStrongestId, std::strong_ordering>);
	static_assert (std::three_way_comparable<GlobalStrongestId, std::strong_ordering>);

	namespace
	{
		constexpr quint8 EntryIdVersion = 1;
		constexpr quint8 GlobalIdVersion = 1;
		constexpr quint8 StrongestIdVersion = 1;

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

		quint8 KindToByte (IdKind kind)
		{
			return static_cast<quint8> (kind);
		}

		std::optional<IdKind> KindFromByte (quint8 byte)
		{
			switch (const auto kind = static_cast<IdKind> (byte))
			{
			case IdKind::Persistent:
			case IdKind::Conventional:
				return kind;
			}
			return {};
		}

		template<IdKind K>
		QString ToSerializedString (const EntryId<K>& id)
		{
			return id.ToString ();
		}

		template<IdKind K>
		QString ToSerializedString (const GlobalId<K>& id)
		{
			const auto& accIdStr = id.AccountId_.ToString ();
			return QString::number (accIdStr.size ()) + ':' + accIdStr + ':' + ToSerializedString (id.EntryId_);
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

			return GlobalId<K> { AccountId::FromString (str.sliced (accStart, accLen).toString ()), *entryId };
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

		template<typename T>
		std::optional<T> ReadRaw (QDataStream& stream, std::source_location loc)
		{
			T value {};
			stream >> value;
			if (stream.status () == QDataStream::Ok)
				return value;

			Util::LogAt (loc).warning () << "corrupt entry id:" << stream.status ();
			return {};
		}

		template<IdKind K>
		void ToStream (QDataStream& stream, const EntryId<K>& id)
		{
			stream << EntryIdVersion << id.ToString ();
		}

		template<IdKind K>
		void ToStream (QDataStream& stream, const GlobalId<K>& id)
		{
			stream << GlobalIdVersion << id.AccountId_.ToString ();
			ToStream (stream, id.EntryId_);
		}

		template<template<IdKind> typename Id>
		void ToStream (QDataStream& stream, const StrongestId<Id>& id)
		{
			std::visit ([&]<IdKind K> (const Id<K>& concrete)
					{
						stream << StrongestIdVersion << KindToByte (K);
						ToStream (stream, concrete);
					},
					id.ToVariant ());
		}

		template<IdKind K>
		std::optional<EntryId<K>> FromStream (std::type_identity<EntryId<K>>, QDataStream& stream, std::source_location loc)
		{
			if (!Util::EnsureVersion<quint8> (stream, 1, EntryIdVersion, loc))
				return {};

			const auto str = ReadRaw<QString> (stream, loc);
			if (!str)
				return {};

			return EntryId<K>::FromString (*str);
		}

		template<IdKind K>
		std::optional<GlobalId<K>> FromStream (std::type_identity<GlobalId<K>>, QDataStream& stream, std::source_location loc)
		{
			if (!Util::EnsureVersion<quint8> (stream, 1, GlobalIdVersion, loc))
				return {};

			const auto accountId = ReadRaw<QString> (stream, loc);
			if (!accountId)
				return {};

			const auto entryId = FromStream (std::type_identity<EntryId<K>> {}, stream, loc);
			if (!entryId)
				return {};

			return GlobalId<K> { AccountId::FromString (*accountId), *entryId };
		}

		template<template<IdKind> typename Id>
		std::optional<StrongestId<Id>> FromStream (std::type_identity<StrongestId<Id>>, QDataStream& stream, std::source_location loc)
		{
			if (!Util::EnsureVersion<quint8> (stream, 1, StrongestIdVersion, loc))
				return {};

			const auto kindByte = ReadRaw<quint8> (stream, loc);
			if (!kindByte)
				return {};

			const auto kind = KindFromByte (*kindByte);
			if (!kind)
			{
				Util::LogAt (loc).warning () << "unknown entry id kind:" << *kindByte;
				stream.setStatus (QDataStream::ReadCorruptData);
				return {};
			}

			switch (*kind)
			{
			case IdKind::Persistent:
				return FromStream (std::type_identity<Id<IdKind::Persistent>> {}, stream, loc);
			case IdKind::Conventional:
				return FromStream (std::type_identity<Id<IdKind::Conventional>> {}, stream, loc);
			}
			std::unreachable ();
		}
	}

	template<AnyEntryId Id>
	void Serialize (QDataStream& stream, const Id& id)
	{
		ToStream (stream, id);
	}

	template<AnyEntryId Id>
	std::optional<Id> Deserialize (QDataStream& stream, std::source_location loc)
	{
		return FromStream (std::type_identity<Id> {}, stream, loc);
	}

	template<AnyEntryId Id>
	QByteArray SerializeBytes (const Id& id)
	{
		QByteArray result;
		QDataStream stream { &result, QDataStream::WriteOnly };
		Serialize (stream, id);
		return result;
	}

	template<AnyEntryId Id>
	std::optional<Id> Deserialize (const QByteArray& data, std::source_location loc)
	{
		QDataStream stream { data };
		const auto result = Deserialize<Id> (stream, loc);
		if (result && !stream.atEnd ())
		{
			Util::LogAt (loc).warning () << "trailing bytes after entry id:" << stream.device ()->bytesAvailable ();
			return {};
		}
		return result;
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

	template<AnyId Id>
	std::optional<Id> IdFromVariant (const QVariant& var)
	{
		if (const auto id = get_if<Id> (&var))
			return *id;
		return {};
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

	template AZOTH_UTIL_API void Serialize (QDataStream&, const EntryPersistentId&);
	template AZOTH_UTIL_API void Serialize (QDataStream&, const EntryConventionalId&);
	template AZOTH_UTIL_API void Serialize (QDataStream&, const GlobalPersistentId&);
	template AZOTH_UTIL_API void Serialize (QDataStream&, const GlobalConventionalId&);
	template AZOTH_UTIL_API void Serialize (QDataStream&, const EntryStrongestId&);
	template AZOTH_UTIL_API void Serialize (QDataStream&, const GlobalStrongestId&);

	template AZOTH_UTIL_API std::optional<EntryPersistentId> Deserialize<EntryPersistentId> (QDataStream&, std::source_location);
	template AZOTH_UTIL_API std::optional<EntryConventionalId> Deserialize<EntryConventionalId> (QDataStream&, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalPersistentId> Deserialize<GlobalPersistentId> (QDataStream&, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalConventionalId> Deserialize<GlobalConventionalId> (QDataStream&, std::source_location);
	template AZOTH_UTIL_API std::optional<EntryStrongestId> Deserialize<EntryStrongestId> (QDataStream&, std::source_location);
	template AZOTH_UTIL_API std::optional<GlobalStrongestId> Deserialize<GlobalStrongestId> (QDataStream&, std::source_location);

	template AZOTH_UTIL_API std::optional<AccountId> IdFromVariant<AccountId> (const QVariant&);
	template AZOTH_UTIL_API std::optional<EntryPersistentId> IdFromVariant<EntryPersistentId> (const QVariant&);
	template AZOTH_UTIL_API std::optional<EntryConventionalId> IdFromVariant<EntryConventionalId> (const QVariant&);
	template AZOTH_UTIL_API std::optional<GlobalPersistentId> IdFromVariant<GlobalPersistentId> (const QVariant&);
	template AZOTH_UTIL_API std::optional<GlobalConventionalId> IdFromVariant<GlobalConventionalId> (const QVariant&);
	template AZOTH_UTIL_API std::optional<EntryStrongestId> IdFromVariant<EntryStrongestId> (const QVariant&);
	template AZOTH_UTIL_API std::optional<GlobalStrongestId> IdFromVariant<GlobalStrongestId> (const QVariant&);
}
