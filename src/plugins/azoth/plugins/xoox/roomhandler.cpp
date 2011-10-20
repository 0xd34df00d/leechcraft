/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QMessageBox>
#include <QInputDialog>
#include <QtDebug>
#include <QXmppVCardIq.h>
#include <QXmppMucManager.h>
#include <QXmppClient.h>
#include <QXmppConstants.h>
#include <interfaces/iproxyobject.h>
#include "glooxaccount.h"
#include "roomclentry.h"
#include "roompublicmessage.h"
#include "roomparticipantentry.h"
#include "clientconnection.h"
#include "glooxmessage.h"
#include "util.h"
#include "glooxprotocol.h"
#include "formbuilder.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	RoomHandler::RoomHandler (const QString& jid,
			const QString& ourNick,
			GlooxAccount* account)
	: Account_ (account)
	, MUCManager_ (Account_->GetClientConnection ()->GetMUCManager ())
	, Room_ (MUCManager_->addRoom (jid))
	, CLEntry_ (new RoomCLEntry (this, Account_))
	{
		Room_->setNickName (ourNick);

		connect (Room_,
				SIGNAL (participantChanged (const QString&)),
				this,
				SLOT (handleParticipantChanged (const QString&)));
		connect (Room_,
				SIGNAL (participantAdded (const QString&)),
				this,
				SLOT (handleParticipantAdded (const QString&)));
		connect (Room_,
				SIGNAL (participantRemoved (const QString&)),
				this,
				SLOT (handleParticipantRemoved (const QString&)));

		connect (this,
				SIGNAL (gotPendingForm (QXmppDataForm*, const QString&)),
				Account_->GetClientConnection ().get (),
				SLOT (handlePendingForm (QXmppDataForm*, const QString&)),
				Qt::QueuedConnection);

		Room_->join ();
	}

	QString RoomHandler::GetRoomJID () const
	{
		return Room_->jid ();
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
		if (state.State_ == SOffline)
		{
			Leave (state.Status_);
			return;
		}

		QXmppPresence pres;
		pres.setTo (GetRoomJID ());
		pres.setType (QXmppPresence::Available);
		pres.setStatus (QXmppPresence::Status (static_cast<QXmppPresence::Status::Type> (state.State_),
				state.Status_,
				state.Priority_));
		Account_->GetClientConnection ()->GetClient ()->sendPacket (pres);
	}

	/** @todo Detect kicks, bans and the respective actor.
	 */
	void RoomHandler::MakeLeaveMessage (const QXmppPresence& pres, const QString& nick)
	{
		QString msg = tr ("%1 has left the room").arg (nick);
		if (pres.status ().statusText ().size ())
			msg += ": " + pres.status ().statusText ();

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantLeave,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	/** @todo Detect the role, affiliation and real jid, if applicable.
	 */
	void RoomHandler::MakeJoinMessage (const QXmppPresence& pres, const QString& nick)
	{
		QString affiliation = XooxUtil::AffiliationToString (pres.mucItem ().affiliation ());
		QString role = XooxUtil::RoleToString (pres.mucItem ().role ());
		QString realJid = pres.mucItem ().jid ();
		QString msg;
		if (realJid.isEmpty ())
			msg = tr ("%1 joined the room as %2 and %3")
					.arg (nick)
					.arg (role)
					.arg (affiliation);
		else
			msg = tr ("%1 (%2) joined the room as %3 and %4")
					.arg (nick)
					.arg (realJid)
					.arg (role)
					.arg (affiliation);

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantJoin,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeStatusChangedMessage (const QXmppPresence& pres, const QString& nick)
	{
		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		IProxyObject *proxy = qobject_cast<IProxyObject*> (proto->GetProxyObject ());

		const QXmppPresence::Status& status = pres.status ();
		const QString& state = proxy->
				StateToString (static_cast<State> (status.type ()));
		QString msg = tr ("%1 changed status to %2 (%3)")
				.arg (nick)
				.arg (state)
				.arg (status.statusText ());

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantStatusChange,
				GetParticipantEntry (nick));
		message->setProperty ("Azoth/Nick", nick);
		message->setProperty ("Azoth/TargetState", state);
		message->setProperty ("Azoth/StatusText", status.statusText ());
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
				IMessage::MSTParticipantNickChange,
				GetParticipantEntry (oldNick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeKickMessage (const QString& nick, const QString& reason)
	{
		QString msg;
		if (reason.isEmpty ())
			msg = tr ("%1 has been kicked")
					.arg (nick);
		else
			msg = tr ("%1 has been kicked: %2")
					.arg (nick)
					.arg (reason);

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTKickNotification,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeBanMessage (const QString& nick, const QString& reason)
	{
		QString msg;
		if (reason.isEmpty ())
			msg = tr ("%1 has been banned")
					.arg (nick);
		else
			msg = tr ("%1 has been banned: %2")
					.arg (nick)
					.arg (reason);

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTBanNotification,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakePermsChangedMessage (const QString& nick,
			QXmppMucItem::Affiliation aff,
			QXmppMucItem::Role role, const QString& reason)
	{
		const QString& affStr = XooxUtil::AffiliationToString (aff);
		const QString& roleStr = XooxUtil::RoleToString (role);
		QString msg;
		if (reason.isEmpty ())
			msg = tr ("%1 is now %2 and %3")
					.arg (nick)
					.arg (roleStr)
					.arg (affStr);
		else
			msg = tr ("%1 is now %2 and %3: %4")
					.arg (nick)
					.arg (roleStr)
					.arg (affStr)
					.arg (reason);

		RoomPublicMessage *message = new RoomPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantRoleAffiliationChange,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::HandleNickConflict ()
	{
		// The room is already joined, should do nothing special here.
		if (Room_->isJoined ())
			return;

		emit CLEntry_->nicknameConflict (Room_->nickName ());
	}

	void RoomHandler::HandlePasswordRequired ()
	{
		bool ok = false;
		const QString& pass = QInputDialog::getText (0,
				tr ("Authorization required"),
				tr ("This room is password-protected. Please enter the "
					"password required to join this room."),
				QLineEdit::Normal,
				QString (),
				&ok);
		if (!ok ||
			pass.isEmpty ())
		{
			Leave (QString ());
			return;
		}

		Room_->setPassword (pass);
		Join ();
	}

	void RoomHandler::HandleErrorPresence (const QXmppPresence& pres, const QString& nick)
	{
		const QString& errorText = pres.error ().text ();
		QString hrText;
		switch (pres.error ().condition ())
		{
		case QXmppStanza::Error::Conflict:
			hrText = tr ("nickname already taken");
			break;
		case QXmppStanza::Error::Forbidden:
		case QXmppStanza::Error::NotAllowed:
			hrText = tr ("access forbidden");
			break;
		case QXmppStanza::Error::NotAuthorized:
			hrText = tr ("password required");
			break;
		case QXmppStanza::Error::JidMalformed:
			hrText = tr ("malformed JID");
			break;
		case QXmppStanza::Error::RegistrationRequired:
			hrText = tr ("only registered users can enter this room");
			break;
		case QXmppStanza::Error::RemoteServerNotFound:
			hrText = tr ("remote server not found (try contacting your server's administrator)");
			break;
		case QXmppStanza::Error::RemoteServerTimeout:
			hrText = tr ("timeout connecting to remote server (try contacting your server's administrator)");
			break;
		default:
			hrText = tr ("unknown condition %1 (please report to developers)")
				.arg (pres.error ().condition ());
			break;
		}
		const QString& text = tr ("Error for %1: %2 (original message: %3)")
				.arg (nick)
				.arg (hrText)
				.arg (errorText.isEmpty () ?
						tr ("no message") :
						errorText);
		RoomPublicMessage *message = new RoomPublicMessage (text,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTEventMessage,
				IMessage::MSTOther);
		CLEntry_->HandleMessage (message);

		switch (pres.error ().condition ())
		{
		case QXmppStanza::Error::Conflict:
			HandleNickConflict ();
			break;
		case QXmppStanza::Error::NotAuthorized:
			HandlePasswordRequired ();
			break;
		default:
			break;
		}
	}

	void RoomHandler::HandlePermsChanged (const QString& nick,
			QXmppMucItem::Affiliation aff,
			QXmppMucItem::Role role,
			const QString& reason)
	{
		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);
		if (aff == QXmppMucItem::OutcastAffiliation ||
			role == QXmppMucItem::NoRole)
		{
			Account_->handleEntryRemoved (entry.get ());

			if (aff == QXmppMucItem::OutcastAffiliation)
				MakeBanMessage (nick, reason);
			else
				MakeKickMessage (nick, reason);

			Nick2Entry_.remove (nick);

			return;
		}

		entry->SetAffiliation (aff);
		entry->SetRole (role);
		MakePermsChangedMessage (nick, aff, role, reason);
	}

	void RoomHandler::HandleMessage (const QXmppMessage& msg, const QString& nick)
	{
		Q_FOREACH (const QXmppElement& elem, msg.extensions ())
		{
			const QString& xmlns = elem.attribute ("xmlns");
			if (xmlns == ns_data)
			{
				QXmppDataForm *df = new QXmppDataForm ();
				df->parse (XooxUtil::XmppElem2DomElem (elem));
				if (df->isNull ())
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to parse form from"
							<< msg.from ();
					delete df;
				}
				else
					emit gotPendingForm (df, msg.from ());
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "unhandled <x> element"
						<< xmlns;
		}

		const bool existed = Nick2Entry_.contains (nick);
		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick, false);
		if (msg.type () == QXmppMessage::Chat && !nick.isEmpty ())
		{
			if (msg.isAttention ())
				entry->HandleAttentionMessage (msg);

			if (msg.state ())
				entry->UpdateChatState (msg.state (), QString ());

			if (!msg.body ().isEmpty ())
			{
				GlooxMessage *message = new GlooxMessage (msg,
						Account_->GetClientConnection ().get ());
				entry->HandleMessage (message);
			}
		}
		else
		{
			RoomPublicMessage *message = 0;
			if (msg.type () == QXmppMessage::GroupChat &&
				!msg.subject ().isEmpty ())
			{
				Subject_ = msg.subject ();
				CLEntry_->HandleSubjectChanged (Subject_);

				const QString& string = nick.isEmpty () ?
						msg.subject () :
						tr ("%1 changed subject to %2")
							.arg (nick)
							.arg (msg.subject ());

				message = new RoomPublicMessage (string,
					IMessage::DIn,
					CLEntry_,
					IMessage::MTEventMessage,
					IMessage::MSTOther);
			}
			else if (!nick.isEmpty ())
			{
				if (!msg.body ().isEmpty ())
					message = new RoomPublicMessage (msg, CLEntry_, entry);
			}
			else if (!msg.body ().isEmpty ())
				message = new RoomPublicMessage (msg.body (),
					IMessage::DIn,
					CLEntry_,
					IMessage::MTEventMessage,
					IMessage::MSTOther);

			if (message)
				CLEntry_->HandleMessage (message);

			if (!existed)
				Nick2Entry_.remove (nick);
		}
	}

	void RoomHandler::UpdatePerms (const QList<QXmppMucItem>& perms)
	{
		Q_FOREACH (const QXmppMucItem& item, perms)
		{
			if (!Nick2Entry_.contains (item.nick ()))
			{
				qWarning () << Q_FUNC_INFO
						<< "no participant with nick"
						<< item.nick ()
						<< Nick2Entry_.keys ();
				continue;
			}

			Nick2Entry_ [item.nick ()]->SetAffiliation (item.affiliation ());
			Nick2Entry_ [item.nick ()]->SetRole (item.role ());
		}
	}

	GlooxMessage* RoomHandler::CreateMessage (IMessage::MessageType,
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

	void RoomHandler::Join ()
	{
		if (Room_->isJoined ())
			return;

		Room_->join ();
	}

	void RoomHandler::SetSubject (const QString& subj)
	{
		Room_->setSubject (subj);
	}

	void RoomHandler::Leave (const QString& msg, bool remove)
	{
		Q_FOREACH (RoomParticipantEntry_ptr entry, Nick2Entry_.values ())
			Account_->handleEntryRemoved (entry.get ());

		Room_->leave (msg);
		Nick2Entry_.clear ();

		if (remove)
			RemoveThis ();
	}

	RoomParticipantEntry* RoomHandler::GetSelf () const
	{
		Q_FOREACH (QObject *partObj, GetParticipants ())
		{
			RoomParticipantEntry *part =
					qobject_cast<RoomParticipantEntry*> (partObj);
			if (part->GetEntryName () == Room_->nickName ())
				return part;
		}
		return 0;
	}

	QString RoomHandler::GetOurNick () const
	{
		return Room_->nickName ();
	}

	void RoomHandler::SetOurNick (const QString& nick)
	{
		Room_->setNickName (nick);
	}

	void RoomHandler::SetAffiliation (RoomParticipantEntry *entry,
			QXmppMucItem::Affiliation newAff, const QString& reason)
	{
		QXmppMucItem item;
		item.setNick (entry->GetNick ());
		item.setReason (reason);
		item.setAffiliation (newAff);
		Account_->GetClientConnection ()->Update (item, Room_->jid ());
	}

	void RoomHandler::SetRole (RoomParticipantEntry *entry,
			QXmppMucItem::Role newRole, const QString& reason)
	{
		QXmppMucItem item;
		item.setNick (entry->GetNick ());
		item.setReason (reason);
		item.setRole (newRole);
		Account_->GetClientConnection ()->Update (item, Room_->jid ());
	}

	QXmppMucRoom* RoomHandler::GetRoom () const
	{
		return Room_;
	}

	RoomParticipantEntry_ptr RoomHandler::CreateParticipantEntry (const QString& nick, bool announce)
	{
		RoomParticipantEntry_ptr entry (new RoomParticipantEntry (nick,
					this, Account_));
		connect (entry.get (),
				SIGNAL (messagesAreRead ()),
				this,
				SLOT (handleMessagesAreRead ()));
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

	void RoomHandler::handleParticipantAdded (const QString& jid)
	{
		const QXmppPresence& pres = Room_->participantPresence (jid);

		QString nick;
		ClientConnection::Split (jid, 0, &nick);

		if (PendingNickChanges_.remove (nick))
			return;

		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);
		entry->SetAffiliation (pres.mucItem ().affiliation ());
		entry->SetRole (pres.mucItem ().role ());
		const QXmppPresence::Status& xmppSt = pres.status ();
		entry->SetStatus (EntryStatus (static_cast<State> (xmppSt.type ()),
					xmppSt.statusText ()),
				QString ());
		entry->SetClientInfo ("", pres);

		Account_->GetClientConnection ()->FetchVCard (jid);
		MakeJoinMessage (pres, nick);
	}

	void RoomHandler::handleParticipantChanged (const QString& jid)
	{
		const QXmppPresence& pres = Room_->participantPresence (jid);

		QString nick;
		ClientConnection::Split (jid, 0, &nick);

		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);

		const QXmppPresence::Status& xmppSt = pres.status ();
		EntryStatus status (static_cast<State> (xmppSt.type ()),
				xmppSt.statusText ());
		if (status != entry->GetStatus (QString ()))
		{
			entry->SetStatus (status, QString ());
			MakeStatusChangedMessage (pres, nick);
		}

		const QXmppMucItem& item = pres.mucItem ();
		if (item.affiliation () != entry->GetAffiliation () ||
				item.role () != entry->GetRole ())
			HandlePermsChanged (nick,
					item.affiliation (), item.role (), item.reason ());
	}

	void RoomHandler::handleParticipantRemoved (const QString& jid)
	{
		const QXmppPresence& pres = Room_->participantPresence (jid);

		QString nick;
		ClientConnection::Split (jid, 0, &nick);

		const bool us = Room_->nickName () == nick;

		RoomParticipantEntry_ptr entry = GetParticipantEntry (nick);
		const QXmppMucItem& item = pres.mucItem ();
		if (!item.nick ().isEmpty () &&
				item.nick () != nick)
		{
			entry->SetEntryName (item.nick ());
			Nick2Entry_ [item.nick ()] = Nick2Entry_ [nick];
			MakeNickChangeMessage (nick, item.nick ());
			Nick2Entry_.remove (nick);
			PendingNickChanges_ << item.nick ();
			return;
		}
		else if (pres.mucStatusCodes ().contains (301))
			!us ?
				MakeBanMessage (nick, item.reason ()) :
				static_cast<void> (QMetaObject::invokeMethod (CLEntry_,
							"beenBanned",
							Qt::QueuedConnection,
							Q_ARG (QString, item.reason ())));
		else if (pres.mucStatusCodes ().contains (307))
			!us ?
				MakeKickMessage (nick, item.reason ()) :
				static_cast<void> (QMetaObject::invokeMethod (CLEntry_,
							"beenKicked",
							Qt::QueuedConnection,
							Q_ARG (QString, item.reason ())));
		else
			MakeLeaveMessage (pres, nick);

		if (us)
		{
			Leave (QString (), false);
			return;
		}

		if (entry->HasUnreadMsgs ())
			entry->SetStatus (EntryStatus (SOffline, item.reason ()), QString ());
		else
			RemoveEntry (entry.get ());
	}

	void RoomHandler::requestVoice ()
	{
		QList<QXmppDataForm::Field> fields;

		QXmppDataForm::Field typeField (QXmppDataForm::Field::HiddenField);
		typeField.setKey ("FORM_TYPE");
		typeField.setValue ("http://jabber.org/protocol/muc#request");
		fields << typeField;

		QXmppDataForm::Field reqField (QXmppDataForm::Field::TextSingleField);
		reqField.setLabel ("Requested role");
		reqField.setKey ("muc#role");
		reqField.setValue ("participant");
		fields << reqField;

		QXmppDataForm form;
		form.setType (QXmppDataForm::Submit);
		form.setFields (fields);

		QXmppMessage msg ("", Room_->jid ());
		msg.setType (QXmppMessage::Normal);
		msg.setExtensions (XooxUtil::Form2XmppElem (form));

		Account_->GetClientConnection ()->GetClient ()->sendPacket (msg);
	}

	void RoomHandler::handleMessagesAreRead ()
	{
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a RoomParticipantEntry";
			return;
		}

		if (entry->GetStatus (QString ()).State_ == SOffline)
			RemoveEntry (entry);
	}

	void RoomHandler::RemoveEntry (RoomParticipantEntry *entry)
	{
		Nick2Entry_.remove (entry->GetNick ());
		Account_->handleEntryRemoved (entry);
	}

	void RoomHandler::RemoveThis ()
	{
		Account_->handleEntryRemoved (CLEntry_);
		Account_->GetClientConnection ()->Unregister (this);
		deleteLater ();
	}
}
}
}
