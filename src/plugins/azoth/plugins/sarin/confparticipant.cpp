/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "confparticipant.h"
#include <util/sll/visitor.h>
#include <util/util.h>
#include "confentry.h"
#include "confsmanager.h"
#include "toxaccount.h"
#include "util.h"

namespace LC::Azoth::Sarin
{
	ConfParticipant::ConfParticipant (Pubkey pkey, QString nick, State state, ConfEntry& conf, QObject *parent)
	: QObject { parent }
	, Acc_ { conf.GetConfsManager ().GetAccount () }
	, Conf_ { conf }
	, Pkey_ { pkey }
	, EntryId_ { Acc_.GetAccountID () + '_' + ToxId2HR (pkey) }
	, Nick_ { std::move (nick) }
	, State_ { std::move (state) }
	{
	}

	QObject* ConfParticipant::GetQObject ()
	{
		return this;
	}

	IAccount* ConfParticipant::GetParentAccount () const
	{
		return &Acc_;
	}

	ConfEntry* ConfParticipant::GetParentCLEntry () const
	{
		return &Conf_;
	}

	ICLEntry::Features ConfParticipant::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType ConfParticipant::GetEntryType () const
	{
		return EntryType::PrivateChat;
	}

	QString ConfParticipant::GetEntryName () const
	{
		return Nick_;
	}

	void ConfParticipant::SetEntryName (const QString&)
	{
		qWarning () << "renaming isn't supported";
	}

	QString ConfParticipant::GetEntryID () const
	{
		return EntryId_;
	}

	QStringList ConfParticipant::Groups () const
	{
		return { tr ("Members of %1").arg (Conf_.GetEntryName ()) };
	}

	void ConfParticipant::SetGroups (const QStringList&)
	{
		qWarning () << "changing groups isn't supported";
	}

	QStringList ConfParticipant::Variants () const
	{
		return { {} };
	}

	void ConfParticipant::SendMessage (const OutgoingMessage&)
	{
		qWarning () << "private messages aren't supported";
	}

	QList<IMessage*> ConfParticipant::GetAllMessages () const
	{
		return {};
	}

	void ConfParticipant::PurgeMessages (const QDateTime&)
	{
	}

	void ConfParticipant::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus ConfParticipant::GetStatus (const QString&) const
	{
		return Util::Visit (State_,
				[] (Online) { return EntryStatus { SOnline, {} }; },
				[] (const Offline& offline)
				{
					const auto& sinceStr = Util::GetLocale ().toString (offline.Since_);
					return EntryStatus { SOffline, tr ("Offline since %1.").arg (sinceStr) };
				});
	}

	void ConfParticipant::ShowInfo ()
	{
	}

	QList<QAction*> ConfParticipant::GetActions () const
	{
		return {};
	}

	QMap<QString, QVariant> ConfParticipant::GetClientInfo (const QString&) const
	{
		return {};
	}

	void ConfParticipant::MarkMsgsRead ()
	{
	}

	void ConfParticipant::ChatTabClosed ()
	{
	}

	void ConfParticipant::SetState (const State& state)
	{
		if (State_ == state)
			return;
		State_ = state;
		emit statusChanged (GetStatus ({}), {});
	}
}
