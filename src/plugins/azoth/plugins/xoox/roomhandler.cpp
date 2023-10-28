/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "roomhandler.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QtDebug>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTimer>
#include <QXmppVCardIq.h>
#include <QXmppMucManager.h>
#include <QXmppClient.h>
#include <util/sll/util.h>
#include <util/sll/eithercont.h>
#include <util/sll/prelude.h>
#include <util/xpc/passutils.h>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxaccount.h"
#include "roomclentry.h"
#include "roompublicmessage.h"
#include "roomparticipantentry.h"
#include "clientconnection.h"
#include "clientconnectionextensionsmanager.h"
#include "glooxmessage.h"
#include "util.h"
#include "glooxprotocol.h"
#include "formbuilder.h"
#include "sdmanager.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NSData = "jabber:x:data";

	RoomHandler::RoomHandler (const QString& jid,
			const QString& ourNick,
			bool asAutojoin,
			GlooxAccount* account)
	: Account_ (account)
	, MUCManager_ (Account_->GetClientConnection ()->GetMUCManager ())
	, RoomJID_ (jid)
	, Room_ (MUCManager_->addRoom (jid))
	, CLEntry_ (new RoomCLEntry (this, asAutojoin, Account_))
	, HadRequestedPassword_ (false)
	{
		const QString& server = jid.split ('@', Qt::SkipEmptyParts).value (1);
		auto sdManager = Account_->GetClientConnection ()->GetSDManager ();

		QPointer<RoomHandler> pThis (this);
		sdManager->RequestInfo ([pThis] (const QXmppDiscoveryIq& iq)
					{ if (pThis) pThis->ServerDisco_ = iq; },
				server);

		Room_->setNickName (ourNick.isEmpty () ? account->GetOurNick () : ourNick);

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

		QTimer::singleShot (0, this, [this] { Room_->join (); });
	}

	QString RoomHandler::GetRoomJID () const
	{
		return RoomJID_;
	}

	RoomCLEntry* RoomHandler::GetCLEntry ()
	{
		return CLEntry_;
	}

	void RoomHandler::SetPresence (QXmppPresence pres)
	{
		if (pres.type () == QXmppPresence::Unavailable)
			Leave (pres.statusText (), false);
		else if (!Room_->isJoined ())
			Join ();
	}

	void RoomHandler::MakeLeaveMessage (const QXmppPresence& pres, const QString& nick)
	{
		QString msg = tr ("%1 has left the room").arg (nick);
		if (pres.statusText ().size ())
			msg += ": " + pres.statusText ();

		const auto message = new RoomPublicMessage (msg,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantLeave,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeJoinMessage (const QXmppPresence& pres, const QString& nick)
	{
		const auto& affiliation = XooxUtil::AffiliationToString (pres.mucItem ().affiliation ());
		const auto& role = XooxUtil::RoleToString (pres.mucItem ().role ());
		const auto& realJid = pres.mucItem ().jid ();
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

		const auto message = new RoomPublicMessage (msg,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantJoin,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeStatusChangedMessage (const QXmppPresence& pres, const QString& nick)
	{
		const auto proxy = Account_->GetParentProtocol ()->GetProxyObject ();
		const auto& state = proxy->StateToString (static_cast<State> (pres.availableStatusType () + 1));
		const auto& msg = tr ("%1 changed status to %2 (%3)")
				.arg (nick)
				.arg (state)
				.arg (pres.statusText ());

		const auto message = new RoomPublicMessage (msg,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantStatusChange,
				GetParticipantEntry (nick));
		message->setProperty ("Azoth/Nick", nick);
		message->setProperty ("Azoth/TargetState", state);
		message->setProperty ("Azoth/StatusText", pres.statusText ());
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakeNickChangeMessage (const QString& oldNick, const QString& newNick)
	{
		QString msg = tr ("%1 changed nick to %2")
				.arg (oldNick)
				.arg (newNick);

		const auto message = new RoomPublicMessage (msg,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantNickChange,
				GetParticipantEntry (newNick));
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

		const auto message = new RoomPublicMessage (msg,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::StatusMessage,
				IMessage::SubType::KickNotification,
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

		const auto message = new RoomPublicMessage (msg,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::StatusMessage,
				IMessage::SubType::BanNotification,
				GetParticipantEntry (nick));
		CLEntry_->HandleMessage (message);
	}

	void RoomHandler::MakePermsChangedMessage (const QString& nick,
			QXmppMucItem::Affiliation aff,
			QXmppMucItem::Role role, const QString& reason)
	{
		const auto& affStr = XooxUtil::AffiliationToString (aff);
		const auto& roleStr = XooxUtil::RoleToString (role);
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

		const auto message = new RoomPublicMessage (msg,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantRoleAffiliationChange,
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
		const auto& text = tr ("This room is password-protected. Please enter the "
				"password required to join this room.");
		Util::GetPassword (GetPassKey (),
				text,
				Core::Instance ().GetProxy (),
				{
					[this] { Leave ({}); },
					[this] (const QString& pass)
					{
						HadRequestedPassword_ = true;
						Room_->setPassword (pass);
						Join ();
					}
				},
				this,
				!HadRequestedPassword_);
	}

	QString RoomHandler::GetPassKey () const
	{
		return "org.LeechCraft.Azoth.Xoox.MUCpass_" + CLEntry_->GetHumanReadableID ();
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
		case QXmppStanza::Error::ServiceUnavailable:
			hrText = tr ("service unavailable");
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
		const auto message = new RoomPublicMessage (text,
				IMessage::Direction::In,
				CLEntry_,
				IMessage::Type::EventMessage,
				IMessage::SubType::Other);
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
		const auto& entry = GetParticipantEntry (nick);
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

	void RoomHandler::HandleMessageExtensions (const QXmppMessage& msg)
	{
		for (const auto& elem : msg.extensions ())
		{
			const auto& xmlns = elem.attribute ("xmlns");
			if (xmlns == NSData)
			{
				auto df = std::make_unique<QXmppDataForm> ();
				df->parse (XooxUtil::XmppElem2DomElem (elem));
				if (df->isNull ())
					qWarning () << Q_FUNC_INFO
							<< "unable to parse form from"
							<< msg.from ();
				else
					HandlePendingForm (std::move (df), msg.from ());
			}
			else
			{
				QString str;
				QXmlStreamWriter w (&str);
				w.setAutoFormatting (true);
				elem.toXml (&w);
				qWarning () << Q_FUNC_INFO
						<< "unhandled <x> element"
						<< str;
			}
		}
	}

	void RoomHandler::HandlePendingForm (std::unique_ptr<QXmppDataForm> form, const QString& from)
	{
		const auto client = Account_->GetClientConnection ();

		FormBuilder fb { from, &client->Exts ().Get<XMPPBobManager> () };

		QDialog dia;
		dia.setWindowTitle (tr ("Data form from %1").arg (from));
		dia.setLayout (new QVBoxLayout ());

		dia.layout ()->addWidget (new QLabel { tr ("You have received dataform from %1:").arg (from) });
		dia.layout ()->addWidget (fb.CreateForm (*form));
		auto box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect (box,
				&QDialogButtonBox::accepted,
				&dia,
				&QDialog::accept);
		connect (box,
				&QDialogButtonBox::rejected,
				&dia,
				&QDialog::reject);
		dia.layout ()->addWidget (box);
		dia.setWindowModality (Qt::WindowModal);
		if (dia.exec () != QDialog::Accepted)
			return;

		QXmppMessage msg ("", from);
		msg.setType (QXmppMessage::Normal);
		auto subForm = fb.GetForm ();
		subForm.setType (QXmppDataForm::Submit);
		msg.setExtensions ({ XooxUtil::Form2XmppElem (subForm) });
		client->GetClient ()->sendPacket (msg);
	}

	void RoomHandler::HandleMessage (const QXmppMessage& msg, const QString& nick)
	{
		HandleMessageExtensions (msg);

		const bool existed = Nick2Entry_.contains (nick);
		const auto& entry = GetParticipantEntry (nick, false);
		if (msg.type () == QXmppMessage::Chat && !nick.isEmpty ())
		{
			if (msg.isAttentionRequested ())
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
			RoomPublicMessage *message = nullptr;
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
					IMessage::Direction::In,
					CLEntry_,
					IMessage::Type::EventMessage,
					IMessage::SubType::RoomSubjectChange);
			}
			else if (!nick.isEmpty ())
			{
				if (!msg.body ().isEmpty ())
					message = new RoomPublicMessage (msg, CLEntry_, entry);
			}
			else if (!msg.body ().isEmpty ())
				message = new RoomPublicMessage (msg.body (),
					IMessage::Direction::In,
					CLEntry_,
					IMessage::Type::EventMessage,
					IMessage::SubType::Other);

			if (message)
				CLEntry_->HandleMessage (message);

			if (!existed)
				RemoveEntry (entry.get ());
		}
	}

	GlooxMessage* RoomHandler::CreateMessage (IMessage::Type,
			const QString& nick, const QString& body)
	{
		const auto message = new GlooxMessage (IMessage::Type::ChatMessage,
				IMessage::Direction::Out,
				GetRoomJID (),
				nick,
				Account_->GetClientConnection ().get ());
		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());
		return message;
	}

	QList<QObject*> RoomHandler::GetParticipants () const
	{
		return Util::Map (Nick2Entry_, [] (const auto& ptr) -> QObject* { return ptr.get (); });
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
		for (const auto& entry : Nick2Entry_)
			Account_->handleEntryRemoved (entry.get ());
		if (RoomPseudoEntry_)
			Account_->handleEntryRemoved (RoomPseudoEntry_.get ());

		Room_->leave (msg);
		Nick2Entry_.clear ();
		RoomPseudoEntry_.reset ();

		if (remove)
			RemoveThis ();
	}

	RoomParticipantEntry* RoomHandler::GetSelf ()
	{
		return GetParticipantEntry (Room_->nickName ()).get ();
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

	void RoomHandler::HandleRenameStart (const RoomParticipantEntry_ptr& entry,
			const QString& nick, const QString& newNick)
	{
		if (entry == RoomPseudoEntry_)
		{
			qWarning () << "tried to rename the pseudo entry";
			return;
		}

		if (!Nick2Entry_.contains (newNick))
		{
			const auto& newEntry = GetParticipantEntry (newNick, false);
			newEntry->SetAffiliation (entry->GetAffiliation ());
			newEntry->SetRole (entry->GetRole ());
			Account_->gotCLItems ({ newEntry.get () });
		}

		PendingNickChanges_ << newNick;

		const auto& otherEntry = Nick2Entry_.value (newNick);
		otherEntry->StealMessagesFrom (entry.get ());
		CLEntry_->MoveMessages (entry, otherEntry);

		MakeNickChangeMessage (nick, newNick);
		Account_->handleEntryRemoved (Nick2Entry_.value (nick).get ());
		Nick2Entry_.remove (nick);
	}

	RoomParticipantEntry_ptr RoomHandler::GetParticipantEntry (const QString& nick, bool announce)
	{
		auto& entry = nick.isEmpty () ?
				RoomPseudoEntry_ :
				Nick2Entry_ [nick];

		if (!entry)
		{
			entry = std::make_shared<RoomParticipantEntry> (nick, this, Account_);
			connect (entry.get (),
					&RoomParticipantEntry::chatTabClosed,
					entry.get (),
					[this, entry = entry.get ()]
					{
						if (entry->GetStatus (QString ()).State_ == SOffline)
							RemoveEntry (entry);
					},
					Qt::QueuedConnection);
			if (announce && !nick.isEmpty ())
				Account_->gotCLItems ({ entry.get () });
		}

		return entry;
	}

	void RoomHandler::handleParticipantAdded (const QString& jid)
	{
		const auto& pres = Room_->participantPresence (jid);

		const auto nick = ClientConnection::Split (jid).Resource_;

		const bool existed = Nick2Entry_.contains (nick);

		const auto& entry = GetParticipantEntry (nick, false);

		if (PendingNickChanges_.remove (nick))
		{
			entry->HandlePresence (pres, {});
			return;
		}

		entry->SetAffiliation (pres.mucItem ().affiliation ());
		entry->SetRole (pres.mucItem ().role ());

		entry->HandlePresence (pres, {});

		if (!existed && !nick.isEmpty ())
			Account_->gotCLItems ({ entry.get () });

		MakeJoinMessage (pres, nick);
	}

	void RoomHandler::handleParticipantChanged (const QString& jid)
	{
		const auto& pres = Room_->participantPresence (jid);

		const auto nick = ClientConnection::Split (jid).Resource_;

		const auto& entry = GetParticipantEntry (nick);

		entry->HandlePresence (pres, QString ());
		if (XooxUtil::PresenceToStatus (pres) != entry->GetStatus (QString ()))
			MakeStatusChangedMessage (pres, nick);

		const auto& item = pres.mucItem ();
		if (item.affiliation () != entry->GetAffiliation () ||
				item.role () != entry->GetRole ())
			HandlePermsChanged (nick, item.affiliation (), item.role (), item.reason ());
	}

	void RoomHandler::handleParticipantRemoved (const QString& jid)
	{
		const auto& pres = Room_->participantPresence (jid);

		const auto nick = ClientConnection::Split (jid).Resource_;

		const bool us = Room_->nickName () == nick;

		const auto& entry = GetParticipantEntry (nick);
		const auto& item = pres.mucItem ();
		const auto& reason = item.reason ();
		if (!item.nick ().isEmpty () &&
				item.nick () != nick)
		{
			HandleRenameStart (entry, nick, item.nick ());
			return;
		}
		else if (pres.mucStatusCodes ().contains (301))
			!us ?
				MakeBanMessage (nick, reason) :
				QTimer::singleShot (0, this, [=, this] { CLEntry_->beenBanned (reason); });
		else if (pres.mucStatusCodes ().contains (307))
			!us ?
				MakeKickMessage (nick, reason) :
				QTimer::singleShot (0, this, [=, this] { CLEntry_->beenKicked (reason); });
		else
			MakeLeaveMessage (pres, nick);

		const auto checkRejoin = Util::MakeScopeGuard ([this]
				{
					if (Nick2Entry_.isEmpty () ||
							std::all_of (Nick2Entry_.begin (), Nick2Entry_.end (),
									[] (const RoomParticipantEntry_ptr& entry)
										{ return entry->GetStatus ({}).State_ == SOffline; }))
						QTimer::singleShot (5000, this, &RoomHandler::Join);
				});

		if (us)
		{
			Leave ({}, false);
			return;
		}

		if (entry->HasUnreadMsgs ())
			entry->SetStatus (EntryStatus (SOffline, reason),
					QString (), QXmppPresence (QXmppPresence::Unavailable));
		else
			RemoveEntry (entry.get ());
	}

	void RoomHandler::requestVoice ()
	{
		QXmppDataForm::Field typeField (QXmppDataForm::Field::HiddenField);
		typeField.setKey ("FORM_TYPE");
		typeField.setValue ("http://jabber.org/protocol/muc#request");

		QXmppDataForm::Field reqField (QXmppDataForm::Field::TextSingleField);
		reqField.setLabel ("Requested role");
		reqField.setKey ("muc#role");
		reqField.setValue ("participant");

		QXmppDataForm form { QXmppDataForm::Submit, { typeField, reqField } };

		QXmppMessage msg ({}, Room_->jid ());
		msg.setType (QXmppMessage::Normal);
		msg.setExtensions ({ XooxUtil::Form2XmppElem (form) });

		Account_->GetClientConnection ()->GetClient ()->sendPacket (msg);
	}

	bool RoomHandler::IsGateway () const
	{
		if (ServerDisco_.identities ().size () != 1)
			return true;

		auto id = ServerDisco_.identities ().at (0);
		return id.category () == "conference" && id.type () != "text";
	}

	void RoomHandler::RemoveEntry (RoomParticipantEntry *entry)
	{
		Account_->handleEntryRemoved (entry);
		if (const auto nick = entry->GetNick ();
			!nick.isEmpty ())
			Nick2Entry_.remove (nick);
		else
			RoomPseudoEntry_.reset ();
	}

	void RoomHandler::RemoveThis ()
	{
		Account_->GetClientConnection ()->Unregister (this);
		Account_->handleEntryRemoved (CLEntry_);
		Room_->deleteLater ();
		Room_ = 0;
		deleteLater ();
	}
}
}
}
