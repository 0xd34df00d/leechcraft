/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "roomhandler.h"
#include <QtDebug>
#include <QXmppVCardIq.h>
#include <QXmppMucManager.h>
#include <QXmppClient.h>
#include <interfaces/iproxyobject.h>
#include "glooxaccount.h"
#include "roomclentry.h"
#include "roompublicmessage.h"
#include "roomparticipantentry.h"
#include "clientconnection.h"
#include "glooxmessage.h"
#include "util.h"
#include "glooxprotocol.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	RoomHandler::RoomHandler (const QString& jid,
			const QString& ourNick, GlooxAccount* account)
	: Account_ (account)
	, CLEntry_ (new RoomCLEntry (this, Account_))
	, RoomHasBeenEntered_ (false)
	, RoomJID_ (jid)
	, OurNick_ (ourNick)
	, MUCManager_ (Account_->GetClientConnection ()->GetMUCManager ())
	{
	}

	QString RoomHandler::GetRoomJID () const
	{
		return RoomJID_;
	}

	RoomCLEntry* RoomHandler::GetCLEntry ()
	{
		return CLEntry_;
	}

	void RoomHandler::HandleVCard (const QXmppVCardIq& card, const QString& nick)
	{
		if (!Nick2Entry_.contains (nick))
		{
			qWarning () << Q_FUNC_INFO
					<< "no such nick"
					<< nick
					<< "; available:"
					<< Nick2Entry_.keys ();
			return;
		}

		Nick2Entry_ [nick]->SetAvatar (card.photo ());
		Nick2Entry_ [nick]->SetVCard (card);
	}

	void RoomHandler::SetState (const GlooxAccountState& state)
	{
		QXmppPresence pres;
		pres.setTo (GetRoomJID ());
		pres.setType (QXmppPresence::Available);
		pres.setStatus (QXmppPresence::Status (static_cast<QXmppPresence::Status::Type> (state.State_),
				state.Status_,
				state.Priority_));
		Account_->GetClientConnection ()->GetClient ()->sendPacket (pres);
	}

	/*
	void RoomHandler::MakeLeaveMessage (const gloox::MUCRoomParticipant part)
	{
		const QString& nick = NickFromJID (*part.nick);
		QString msg = tr ("%1 has left the room").arg (nick);

		QString actor;
		if (part.actor)
			actor = NickFromJID (*part.actor);

		if (part.flags & gloox::UserKicked)
			msg += actor.isEmpty () ?
				tr (" (kicked)") :
				tr (" (kicked by %1)")
					.arg (actor);
		if (part.flags & gloox::UserBanned)
			msg += actor.isEmpty () ?
				tr (" (banned)") :
				tr (" (banned by %1)")
					.arg (actor);

		if (part.reason.size ())
		{
			msg += ": ";
			msg += QString::fromUtf8 (part.reason.c_str ());
		}

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantLeave);
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeStatusChangedMessage (const gloox::MUCRoomParticipant part,
			const gloox::Presence& presence)
	{
		const QString& nick = NickFromJID (*part.nick);
		State state = static_cast<State> (presence.presence ());

		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		IProxyObject *proxy = qobject_cast<IProxyObject*> (proto->GetProxyObject ());

		QString msg = tr ("%1 changed status to %2")
			.arg (nick)
			.arg (proxy->StateToString (state));

		if (part.status.size ())
			msg += QString (" (%1)")
				.arg (QString::fromUtf8 (part.status.c_str ()));

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantStatusChange);
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeRoleAffChangedMessage (const gloox::MUCRoomParticipant part)
	{
		const QString& nick = NickFromJID (*part.nick);

		const QString& byStr = part.actor ?
				tr ("by %1").arg (NickFromJID (*part.actor)) :
				QString ();

		const QString& msg = tr ("%1 changed affiliation/role to %2/%3 %4")
				.arg (nick)
				.arg (Util::AffiliationToString (part.affiliation))
				.arg (Util::RoleToString (part.role))
				.arg (byStr);

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantRoleAffiliationChange);
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeJoinMessage (const gloox::MUCRoomParticipant part)
	{
		const QString& nick = NickFromJID (*part.nick);
		const QString& role = Util::RoleToString (part.role);
		const QString& aff = Util::AffiliationToString (part.affiliation);
		QString msg = tr ("%1 joined the room as %2 and %3")
				.arg (nick)
				.arg (role)
				.arg (aff);

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantJoin);
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeNickChangeMessage (const QString& oldNick, const QString& newNick)
	{
		QString msg = tr ("%1 changed nick to %2")
				.arg (oldNick)
				.arg (newNick);

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantNickChange);
		CLEntry_->HandleMessage (message);
	}
	*/

	void RoomHandler::HandlePresence (const QXmppPresence& pres, const QString& nick)
	{
		const bool existed = Nick2Entry_.contains (nick);
		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);

		if (pres.type () == QXmppPresence::Unavailable)
		{
			Account_->handleEntryRemoved (entry.get ());

			Nick2Entry_.remove (nick);

			return;
		}

		const QXmppPresence::Status& xmppSt = pres.status ();
		EntryStatus status (static_cast<State> (xmppSt.type ()),
				xmppSt.statusText ());
		entry->SetStatus (status, QString ());
	}

	/*
	void RoomHandler::handleMUCParticipantPresence (gloox::MUCRoom *room,
			const gloox::MUCRoomParticipant part, const gloox::Presence& presence)
	{
		const QString& nick = NickFromJID (*part.nick);
		const bool existed = Nick2Entry_.contains (nick);
		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);

		const gloox::MUCRoomAffiliation aff = entry->GetAffiliation ();
		const gloox::MUCRoomRole role = entry->GetRole ();
		entry->SetAffiliation (part.affiliation);
		entry->SetRole (part.role);

		const gloox::Capabilities *caps = presence.capabilities ();
		if (caps)
			entry->SetClientInfo ("", caps);

		if (presence.presence () == gloox::Presence::Unavailable)
		{
			if (part.newNick.empty ())
			{
				MakeLeaveMessage (part);

				Account_->handleEntryRemoved (entry.get ());

				Nick2Entry_.remove (nick);
				JID2Session_.remove (JIDForNick (nick));
			}
			else
			{
				const QString& newNick = QString::fromUtf8 (part.newNick.c_str ());

				PendingNickChanges_ << newNick;

				MakeNickChangeMessage (nick, newNick);
				entry->SetEntryName (newNick);
				Nick2Entry_ [newNick] = Nick2Entry_.take (nick);
				JID2Session_ [JIDForNick (newNick)] = JID2Session_.take (JIDForNick (nick));
			}
			return;
		}

		EntryStatus status (static_cast<State> (presence.presence ()),
						QString::fromUtf8 (presence.status ().c_str ()));
		if (RoomHasBeenEntered_)
		{
			if (existed)
			{
				if (PendingNickChanges_.contains (nick))
					PendingNickChanges_.remove (nick);
				else if (entry->GetStatus (QString ()) != status)
					MakeStatusChangedMessage (part, presence);

				if (aff != part.affiliation ||
						role != part.role)
					MakeRoleAffChangedMessage (part);
			}
			else
			{
				MakeJoinMessage (part);
				Account_->GetClientConnection ()->FetchVCard (JIDForNick (nick));
			}
		}

		entry->SetStatus (status, QString ());
	}
	*/
	void RoomHandler::HandleMessage (const QXmppMessage& msg, const QString& nick)
	{
		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick, false);
		if (msg.type () == QXmppMessage::Chat && !nick.isEmpty ())
		{
			GlooxMessage *message = new GlooxMessage (msg,
					Account_->GetClientConnection ().get ());
			entry->HandleMessage (message);
		}
		else
		{
			RoomPublicMessage *message = 0;
			if (!nick.isEmpty ())
				message = new RoomPublicMessage (msg, CLEntry_, entry);
			else
				message = new RoomPublicMessage (msg.body (),
					IMessage::DIn,
					CLEntry_,
					IMessage::MTEventMessage,
					IMessage::MSTOther);
			CLEntry_->HandleMessage (message);
		}
	}

	GlooxMessage* RoomHandler::CreateMessage (IMessage::MessageType type,
			const QString& nick, const QString& body)
	{
		GlooxMessage *message = new GlooxMessage (IMessage::MTChatMessage,
				IMessage::DOut,
				GetRoomJID (),
				nick,
				Account_->GetClientConnection ().get ());
		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());
		return message;
	}

	QList<QObject*> RoomHandler::GetParticipants () const
	{
		QList<QObject*> result;
		Q_FOREACH (RoomParticipantEntry_ptr rpe, Nick2Entry_.values ())
			result << rpe.get ();
		return result;
	}

	QString RoomHandler::GetSubject () const
	{
		return Subject_;
	}

	void RoomHandler::SetSubject (const QString& subj)
	{
		MUCManager_->setRoomSubject (GetRoomJID (), subj);
	}

	void RoomHandler::Leave (const QString& msg)
	{
		Q_FOREACH (RoomParticipantEntry_ptr entry, Nick2Entry_.values ())
			Account_->handleEntryRemoved (entry.get ());

		// TODO use msg
		MUCManager_->leaveRoom (GetRoomJID ());

		RemoveThis ();
	}

	RoomParticipantEntry* RoomHandler::GetSelf () const
	{
		Q_FOREACH (QObject *partObj, GetParticipants ())
		{
			RoomParticipantEntry *part =
					qobject_cast<RoomParticipantEntry*> (partObj);
			if (part->GetEntryName () == OurNick_)
				return part;
		}
		return 0;
	}

	QString RoomHandler::GetOurNick () const
	{
		return OurNick_;
	}

	void RoomHandler::SetOurNick (const QString& nick)
	{
		OurNick_ = nick;

		QXmppMucAdminIq::Item item;
		item.setNick (nick);
		Account_->GetClientConnection ()->Update (item);
	}

	void RoomHandler::SetAffiliation (RoomParticipantEntry *entry,
			IMUCEntry::MUCAffiliation newAff, const QString& reason)
	{
		QXmppMucAdminIq::Item item;
		item.setNick (entry->GetNick ());
		item.setReason (reason);
		item.setAffiliation (static_cast<QXmppMucAdminIq::Item::Affiliation> (newAff));
		Account_->GetClientConnection ()->Update (item);
	}

	void RoomHandler::SetRole (RoomParticipantEntry *entry,
			IMUCEntry::MUCRole newRole, const QString& reason)
	{
		QXmppMucAdminIq::Item item;
		item.setNick (entry->GetNick ());
		item.setReason (reason);
		item.setRole (static_cast<QXmppMucAdminIq::Item::Role> (newRole));
		Account_->GetClientConnection ()->Update (item);
	}

	RoomParticipantEntry_ptr RoomHandler::CreateParticipantEntry (const QString& nick, bool announce)
	{
		RoomParticipantEntry_ptr entry (new RoomParticipantEntry (nick,
					this, Account_));
		Nick2Entry_ [nick] = entry;
		if (announce)
			Account_->handleGotRosterItems (QList<QObject*> () << entry.get ());
		return entry;
	}

	RoomParticipantEntry_ptr RoomHandler::GetParticipantEntry (const QString& nick, bool announce)
	{
		if (!Nick2Entry_.contains (nick))
		{
			RoomParticipantEntry_ptr entry (CreateParticipantEntry (nick, announce));
			Nick2Entry_ [nick] = entry;
			return entry;
		}
		else
			return Nick2Entry_ [nick];
	}

	void RoomHandler::RemoveThis ()
	{
		Nick2Entry_.clear ();

		Account_->handleEntryRemoved (CLEntry_);

		Account_->GetClientConnection ()->Unregister (this);

		deleteLater ();
	}

}
}
}
}
}
