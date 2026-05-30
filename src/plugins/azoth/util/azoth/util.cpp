/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QDataStream>
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include <util/sll/qobjectrefcast.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucprotocol.h>
#include <interfaces/azoth/imucjoinwidget.h>

namespace LC::Azoth
{
	bool IsOnline (State st)
	{
		switch (st)
		{
		case SOffline:
		case SError:
		case SConnecting:
		case SInvalid:
			return false;
		default:
			return true;
		}
	}

	QString StateToString (State st)
	{
		switch (st)
		{
		case SOnline:
			return QObject::tr ("Online");
		case SChat:
			return QObject::tr ("Free to chat");
		case SAway:
			return QObject::tr ("Away");
		case SDND:
			return QObject::tr ("Do not disturb");
		case SXA:
			return QObject::tr ("Not available");
		case SOffline:
			return QObject::tr ("Offline");
		default:
			return QObject::tr ("Error");
		}
	}

	void RejoinMuc (const IMUCEntry& entry)
	{
		const auto acc = dynamic_cast<const ICLEntry&> (entry).GetParentAccount ();
		RejoinMuc (*acc, entry.GetIdentifyingData ());
	}

	void RejoinMuc (IAccount& account, const QVariantMap& identifyingData)
	{
		const auto accObj = account.GetQObject ();
		auto& mucProto = qobject_ref_cast<IMUCProtocol> (account.GetParentProtocol ());

		auto mucJoinWidget = mucProto.GetMUCJoinWidget ();
		auto& imjw = qobject_ref_cast<IMUCJoinWidget> (mucJoinWidget);
		imjw.AccountSelected (accObj);
		imjw.SetIdentifyingData (identifyingData);

		QTimer::singleShot (1000,
				[mucJoinWidget, &imjw, accObj]
				{
					imjw.Join (accObj);
					mucJoinWidget->deleteLater ();
				});
	}

	namespace
	{
		constexpr char KindName (IdKind kind)
		{
			return kind == IdKind::Persistent ? 'P' : 'C';
		}
	}

	template<IdKind K>
	QDebug operator<< (QDebug dbg, const EntryId<K>& id)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "EntryId<" << KindName (K) << ">{" << id.Id_ << '}';
		return dbg;
	}

	template<IdKind K>
	QDebug operator<< (QDebug dbg, const GlobalId<K>& id)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "GlobalId<" << KindName (K) << ">{ " << id.AccountId_ << ", " << id.EntryId_.Id_ << " }";
		return dbg;
	}

	template AZOTH_UTIL_API QDebug operator<< (QDebug, const EntryId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const EntryId<IdKind::Conventional>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const GlobalId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const GlobalId<IdKind::Conventional>&);

	template<IdKind K>
	QDataStream& operator<< (QDataStream& stream, const EntryId<K>& id)
	{
		return stream << id.Id_;
	}

	template<IdKind K>
	QDataStream& operator>> (QDataStream& stream, EntryId<K>& id)
	{
		return stream >> id.Id_;
	}

	template<IdKind K>
	QDataStream& operator<< (QDataStream& stream, const GlobalId<K>& id)
	{
		return stream << id.AccountId_ << id.EntryId_;
	}

	template<IdKind K>
	QDataStream& operator>> (QDataStream& stream, GlobalId<K>& id)
	{
		return stream >> id.AccountId_ >> id.EntryId_;
	}

	template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const EntryId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const EntryId<IdKind::Conventional>&);
	template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, EntryId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, EntryId<IdKind::Conventional>&);

	template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const GlobalId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const GlobalId<IdKind::Conventional>&);
	template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, GlobalId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, GlobalId<IdKind::Conventional>&);

	namespace
	{
		template<typename Variant, typename Reader>
		QDataStream& ReadStrongest (QDataStream& stream, Variant& id, Reader&& reader)
		{
			quint8 kindByte;
			stream >> kindByte;
			switch (static_cast<IdKind> (kindByte))
			{
			case IdKind::Persistent:
				id = reader (std::integral_constant<IdKind, IdKind::Persistent> {});
				break;
			case IdKind::Conventional:
				id = reader (std::integral_constant<IdKind, IdKind::Conventional> {});
				break;
			default:
				stream.setStatus (QDataStream::ReadCorruptData);
			}
			return stream;
		}
	}

	template<template<IdKind> typename Id>
	QDataStream& operator<< (QDataStream& stream, const StrongestId<Id>& id)
	{
		std::visit ([&]<IdKind K> (const Id<K>& concrete) { stream << static_cast<quint8> (K) << concrete; },
				id.ToVariant ());
		return stream;
	}

	template<template<IdKind> typename Id>
	QDataStream& operator>> (QDataStream& stream, StrongestId<Id>& id)
	{
		return ReadStrongest (stream, id,
				[&]<IdKind K> (std::integral_constant<IdKind, K>) -> StrongestId<Id>
				{
					Id<K> concrete;
					stream >> concrete;
					return concrete;
				});
	}

	template<template<IdKind> typename Id>
	QDebug operator<< (QDebug dbg, const StrongestId<Id>& id)
	{
		std::visit ([&] (const auto& concrete) { dbg << concrete; }, id.ToVariant ());
		return dbg;
	}

	template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const StrongestId<EntryId>&);
	template AZOTH_UTIL_API QDataStream& operator<< (QDataStream&, const StrongestId<GlobalId>&);
	template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, StrongestId<EntryId>&);
	template AZOTH_UTIL_API QDataStream& operator>> (QDataStream&, StrongestId<GlobalId>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const StrongestId<EntryId>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const StrongestId<GlobalId>&);
}
