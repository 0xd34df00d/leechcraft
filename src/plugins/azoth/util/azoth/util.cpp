/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/qtutil.h>
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

	QString StateToID (State st)
	{
		switch (st)
		{
		case SOffline:
			return "offline"_qs;
		case SOnline:
			return "online"_qs;
		case SAway:
			return "away"_qs;
		case SXA:
			return "xa"_qs;
		case SDND:
			return "dnd"_qs;
		case SChat:
			return "chat"_qs;
		case SInvisible:
			return "invisible"_qs;
		case SProbe:
			return "probe"_qs;
		case SConnecting:
			return "connecting"_qs;
		case SError:
		case SInvalid:
			return "error"_qs;
		}
		return "error"_qs;
	}

	QString FormatNameWithAddress (const ICLEntry& entry)
	{
		const auto& name = entry.GetEntryName ();
		const auto& address = entry.GetHumanReadableAddress ();
		if (name.isEmpty () || name == address)
			return address;
		return name + " (" + address + ")";
	}

	void RejoinMuc (const IMUCEntry& entry)
	{
		const auto acc = entry.AsCLEntry ().GetParentAccount ();
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

	QDebug operator<< (QDebug dbg, const AccountId& accId)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace ().noquote () << "AccountId{" << accId.ToString () << '}';
		return dbg;
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
		dbg.nospace ().noquote () << "EntryId<" << KindName (K) << ">{" << id.ToString () << '}';
		return dbg;
	}

	template<IdKind K>
	QDebug operator<< (QDebug dbg, const GlobalId<K>& id)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace ().noquote () << "GlobalId<" << KindName (K) << ">{ " << id.AccountId_.ToString () << ", " << id.EntryId_.ToString () << " }";
		return dbg;
	}

	template AZOTH_UTIL_API QDebug operator<< (QDebug, const EntryId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const EntryId<IdKind::Conventional>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const GlobalId<IdKind::Persistent>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const GlobalId<IdKind::Conventional>&);

	template<template<IdKind> typename Id>
	QDebug operator<< (QDebug dbg, const StrongestId<Id>& id)
	{
		std::visit ([&] (const auto& concrete) { dbg << concrete; }, id.ToVariant ());
		return dbg;
	}

	template AZOTH_UTIL_API QDebug operator<< (QDebug, const StrongestId<EntryId>&);
	template AZOTH_UTIL_API QDebug operator<< (QDebug, const StrongestId<GlobalId>&);
}
