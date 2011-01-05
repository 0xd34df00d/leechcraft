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
#include <gloox/mucroom.h>
#include <gloox/messagesession.h>
#include <gloox/client.h>
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
	RoomHandler::RoomHandler (GlooxAccount* account)
	: Account_ (account)
	, CLEntry_ (0)
	, RoomHasBeenEntered_ (false)
	{
	}

	void RoomHandler::SetRoom (boost::shared_ptr<gloox::MUCRoom> room)
	{
		Room_ = room;
		CLEntry_ = new RoomCLEntry (this, Account_);
		room->getRoomItems ();
	}

	boost::shared_ptr<gloox::MUCRoom> RoomHandler::GetRoom () const
	{
		return Room_;
	}

	gloox::JID RoomHandler::GetRoomJID () const
	{
		return Room_->name () + "@" + Room_->service ();
	}

	RoomCLEntry* RoomHandler::GetCLEntry ()
	{
		return CLEntry_;
	}

	void RoomHandler::HandleVCard (const gloox::VCard *card, const QString& nick)
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

		Nick2Entry_ [nick]->SetAvatar (card->photo ());
	}

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

	void RoomHandler::handleMUCParticipantPresence (gloox::MUCRoom *room,
			const gloox::MUCRoomParticipant part, const gloox::Presence& presence)
	{
		const QString& nick = NickFromJID (*part.nick);
		const bool existed = Nick2Entry_.contains (nick);
		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);

		if (presence.presence () == gloox::Presence::Unavailable)
		{
			MakeLeaveMessage (part);

			Account_->handleEntryRemoved (entry.get ());
			Nick2Entry_.remove (nick);
			JID2Session_.remove (JIDForNick (nick));
			return;
		}

		if (RoomHasBeenEntered_)
		{
			if (existed)
				MakeStatusChangedMessage (part, presence);
			else
			{
				MakeJoinMessage (part);
				Account_->GetClientConnection ()->FetchVCard (JIDForNick (nick), this);
			}
		}

		EntryStatus status (static_cast<State> (presence.presence ()),
				QString::fromUtf8 (presence.status ().c_str ()));
		entry->SetStatus (status, QString ());
	}

	void RoomHandler::handleMUCMessage (gloox::MUCRoom*, const gloox::Message& msg, bool priv)
	{
		const QString& nick = NickFromJID (msg.from ());
		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick, false);

		if (priv && !nick.isEmpty ())
		{
			gloox::MessageSession *session = GetSessionWith (msg.from ());
			GlooxMessage *message = new GlooxMessage (msg, entry.get (), session);
			message->SetDateTime (QDateTime::currentDateTime ());
			entry->HandleMessage (message);
		}
		else
		{
			RoomPublicMessage *message = 0;
			if (!nick.isEmpty ())
				message = new RoomPublicMessage (msg, CLEntry_, entry);
			else
				message = new RoomPublicMessage (QString::fromUtf8 (msg.body ().c_str ()),
						IMessage::DIn,
						CLEntry_,
						IMessage::MTEventMessage,
						IMessage::MSTOther);
			CLEntry_->HandleMessage (message);
		}
	}

	bool RoomHandler::handleMUCRoomCreation (gloox::MUCRoom *room)
	{
		return true;
	}

	void RoomHandler::handleMUCSubject (gloox::MUCRoom *room,
			const std::string& nickStr, const std::string& subject)
	{
		RoomHasBeenEntered_ = true;

		QString nick = QString::fromUtf8 (nickStr.c_str ());
		Subject_ = QString::fromUtf8 (subject.c_str ());
		QString string;
		if (!nick.isEmpty ())
			string = tr ("%1 set subject to %2")
					.arg (nick)
					.arg (Subject_);
		else
			string = tr ("Room subject is %1")
					.arg (Subject_);

		RoomPublicMessage *message =
				new RoomPublicMessage (string, IMessage::DIn,
						CLEntry_,
						IMessage::MTEventMessage,
						IMessage::MSTRoomSubjectChange);
		CLEntry_->HandleMessage (message);

		CLEntry_->HandleSubjectChanged (Subject_);
	}

	void RoomHandler::handleMUCInviteDecline (gloox::MUCRoom* room, const gloox::JID& invitee, const std::string&reason)
	{
	}

	void RoomHandler::handleMUCError (gloox::MUCRoom* room, gloox::StanzaError error)
	{
	}

	void RoomHandler::handleMUCInfo (gloox::MUCRoom *room, int features, const std::string& name, const gloox::DataForm *infoForm)
	{
	}

	void RoomHandler::handleMUCItems (gloox::MUCRoom *room, const gloox::Disco::ItemList& items)
	{
		QList<ICLEntry*> parts;
		Q_FOREACH (gloox::Disco::Item *item, items)
		{
			const QString nick = QString::fromUtf8 (item->name ().c_str ());
			if (!Nick2Entry_.contains (nick))
				parts << GetParticipantEntry (nick).get ();
		}

		CLEntry_->HandleNewParticipants (parts);
	}

	void RoomHandler::handleMessage (const gloox::Message& msg, gloox::MessageSession *session)
	{
		const gloox::JID& from = msg.from ();
		const QString& nick = NickFromJID (from);
		switch (msg.subtype ())
		{
		case gloox::Message::Groupchat:
		case gloox::Message::Headline:
			handleMUCMessage (Room_.get (), msg, false);
			break;
		default:
		{
			RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);
			GlooxMessage *message = new GlooxMessage (msg,
					entry.get (), session);
			message->SetDateTime (QDateTime::currentDateTime ());
			entry->HandleMessage (message);
		}
		}
	}

	GlooxMessage* RoomHandler::CreateMessage (IMessage::MessageType type,
			const QString& nick, const QString& body)
	{
		GlooxMessage *message = new GlooxMessage (IMessage::MTChatMessage,
				IMessage::DOut,
				GetParticipantEntry (nick).get (),
				GetSessionWith (JIDForNick (nick)));
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

	void RoomHandler::Kick (const QString& nick, const QString& reason)
	{
		Room_->kick (nick.toUtf8 ().constData (),
				reason.toUtf8 ().constData ());
	}

	void RoomHandler::Leave (const QString& msg)
	{
		Q_FOREACH (RoomParticipantEntry_ptr entry, Nick2Entry_.values ())
			Account_->handleEntryRemoved (entry.get ());

		Nick2Entry_.clear ();

		Room_->leave (msg.toUtf8 ().constData ());
		RemoveThis ();
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

	gloox::MessageSession* RoomHandler::GetSessionWith (const gloox::JID& with)
	{
		if (!JID2Session_.contains (with))
		{
			gloox::MessageSession *session =
					new gloox::MessageSession (Account_->GetClientConnection ()->GetClient (),
							with);
			session->registerMessageHandler (this);
			JID2Session_ [with] = session;
			return session;
		}
		else
			return JID2Session_ [with];
	}

	QString RoomHandler::NickFromJID (const gloox::JID& jid) const
	{
		return QString::fromUtf8 (jid.resource ().c_str ());
	}

	gloox::JID RoomHandler::JIDForNick (const QString& nick) const
	{
		boost::shared_ptr<gloox::MUCRoom> room = CLEntry_->GetRoom ();
		return gloox::JID (room->name () + "@" +
				room->service () + "/" +
				nick.toUtf8 ().constData ());
	}

	void RoomHandler::RemoveThis ()
	{
		Account_->handleEntryRemoved (CLEntry_);

		Account_->GetClientConnection ()->Unregister (this);

		Room_.reset ();
		deleteLater ();
	}

}
}
}
}
}
