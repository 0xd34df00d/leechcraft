/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "callshandler.h"
#include <QHostAddress>
#ifdef ENABLE_MEDIACALLS
#include <QXmppCallManager.h>
#endif
#include <util/xpc/util.h>
#include "accountsettingsholder.h"
#include "clientconnection.h"
#include "core.h"
#include "glooxaccount.h"

#ifdef ENABLE_MEDIACALLS
#include "mediacall.h"
#endif

namespace LC::Azoth::Xoox
{
	CallsHandler::CallsHandler (QXmppCallManager& callMgr, ClientConnection& conn, GlooxAccount& acc, QObject *parent)
	: QObject { parent }
	, Mgr_ { callMgr }
	, Conn_ { conn }
	, Acc_ { acc }
	{
#ifdef ENABLE_MEDIACALLS
		connect (&Mgr_,
				&QXmppCallManager::callReceived,
				this,
				[this] (QXmppCall *call)
				{
					const auto entry = qobject_cast<ICLEntry*> (Conn_.GetCLEntry (call->jid ()));
					if (!entry)
					{
						qWarning () << "ignoring call from unknown caller" << call->jid ();
						call->hangup ();
						return;
					}

					emit Acc_.called (new MediaCall (&Acc_, call, *entry));
				});

		auto settings = acc.GetSettings ();
		auto updateSettings = [&callMgr, settings]
		{
			callMgr.setStunServer (QHostAddress (settings->GetStunHost ()), settings->GetStunPort ());
			callMgr.setTurnServer (QHostAddress (settings->GetTurnHost ()), settings->GetTurnPort ());
			callMgr.setTurnUser (settings->GetTurnUser ());
			callMgr.setTurnPassword (settings->GetTurnPass ());
		};
		connect (settings,
				&AccountSettingsHolder::stunSettingsChanged,
				this,
				updateSettings);
		connect (settings,
				&AccountSettingsHolder::turnSettingsChanged,
				this,
				updateSettings);
		updateSettings ();
#else
		Q_UNUSED (Acc_)
		Q_UNUSED (Conn_)
#endif
	}

	QObject* CallsHandler::Call (ICLEntry& entry, const QString& variant)
	{
#ifdef ENABLE_MEDIACALLS
		if (entry.GetQObject () == Acc_.GetSelfContact ())
		{
			Core::Instance ().SendEntity (Util::MakeNotification ("LeechCraft",
					tr ("Why would you call yourself?"),
					Priority::Warning));
			return nullptr;
		}

		auto target = entry.GetConventionalId ().ToString ();

		auto resultingVar = variant;
		if (resultingVar.isEmpty ())
			resultingVar = entry.Variants ().value (0);
		if (!resultingVar.isEmpty ())
			target += '/' + resultingVar;

		const auto call = new MediaCall (&Acc_, Mgr_.call (target), entry);
		emit Acc_.called (call);
		return call;
#else
		Q_UNUSED (entry)
		Q_UNUSED (variant)
		return nullptr;
#endif
	}
}
