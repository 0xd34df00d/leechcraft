/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "roomhandler.h"
#include <QtDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTimer>
#include <QXmppClient.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppMucManager.h>
#include <QXmppTask.h>
#include <QXmppVCardIq.h>
#include <util/sll/eithercont.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/xpc/passutils.h>
#include <util/azoth/util.h>
#include "glooxaccount.h"
#include "roomclentry.h"
#include "roompublicmessage.h"
#include "roomparticipantentry.h"
#include "clientconnection.h"
#include "clientconnectionextensionsmanager.h"
#include "glooxmessage.h"
#include "util.h"
#include "formbuilder.h"

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
		[] (RoomHandler *pthis, QString server) -> Util::ContextTask<void>
		{
			co_await Util::AddContextObject { *pthis };
			const auto discoMgr = pthis->Account_->GetClientConnection ()->GetQXmppDiscoveryManager ();
			const auto eitherReply = Util::EitherFromSwapped (co_await discoMgr->info (server));
			const auto discoInfo = co_await Util::WithHandler (eitherReply,
					[] (const QXmppError& error) { qWarning () << "error requesting server disco info:" << error.description; });
			pthis->IsGateway_ = std::ranges::none_of (discoInfo.identities (),
					[] (const QXmppDiscoIdentity& id) { return id.category () == "conference"_qs && id.type () == "text"_qs; });
		} (this, jid.split ('@', Qt::SkipEmptyParts).value (1));

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
				&QXmppMucRoom::participantRemoved,
				this,
				[this] (const QString& jid)
				{
					const auto& nick = ClientConnection::Split (jid).Resource_;
					const auto& pres = Room_->participantPresence (jid);
					if (const auto newNick = pres.mucItem ().nick ();
						!newNick.isEmpty () && !nick.isEmpty () && newNick != nick)
						HandleRenameStart (GetParticipantEntry (nick), nick, newNick);
					else if (Room_->nickName () == nick)
						HandleSelfRemoved (pres);
					else
						HandleParticipantRemoved (nick, pres);
				});

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
		{
			SendLeave (pres.statusText ());
			RemoveParticipants ();
		}
		else
			Join ();
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

		emit CLEntry_->GetMUCEntryEmitter ().nicknameConflict (Room_->nickName ());
	}

	void RoomHandler::HandlePasswordRequired ()
	{
		const auto& text = tr ("This room is password-protected. Please enter the "
				"password required to join this room.");
		Util::GetPassword (GetPassKey (),
				text,
				GetProxyHolder (),
				{
					[this] { RemoveThis (); },
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
			hrText = tr ("access forbidden");
			break;
		case QXmppStanza::Error::Gone:
			hrText = tr ("the room is no longer available");
			break;
		case QXmppStanza::Error::ItemNotFound:
			hrText = tr ("the room is not found (did the room creator configure it?)");
			break;
		case QXmppStanza::Error::NotAllowed:
			hrText = tr ("action not allowed");
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

		const auto& text = errorText.isEmpty () ?
				tr ("Error for %1: %2").arg (nick, hrText) :
				tr ("Error for %1: %2 (original message: %3)").arg (nick, hrText, errorText);
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
		if (nick.isEmpty ())
			return;

		const auto& entry = GetParticipantEntry (nick);
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
					qWarning () << "unable to parse form from" << msg.from ();
				else
					HandlePendingForm (std::move (df), msg.from ());
			}
			else
			{
				QString str;
				QXmlStreamWriter w (&str);
				w.setAutoFormatting (true);
				elem.toXml (&w);
				qWarning () << "unhandled <x> element" << str;
			}
		}
	}

	void RoomHandler::HandlePendingForm (std::unique_ptr<QXmppDataForm> form, const QString& from)
	{
		const auto client = Account_->GetClientConnection ();

		FormBuilder fb { *form, from, &client->Exts ().Get<XMPPBobManager> () };

		QDialog dia;
		dia.setWindowTitle (tr ("Data form from %1").arg (from));
		dia.setLayout (new QVBoxLayout ());

		dia.layout ()->addWidget (new QLabel { tr ("You have received dataform from %1:").arg (from) });
		dia.layout ()->addWidget (fb.CreateForm ());
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

		QXmppMessage msg ({}, from);
		msg.setType (QXmppMessage::Normal);
		msg.setExtensions ({ XooxUtil::Form2XmppElem (fb.GetUpdatedForm (QXmppDataForm::Submit)) });
		client->GetClient ()->send (std::move (msg));
	}

	void RoomHandler::HandleMessage (const QXmppMessage& msg, const QString& nick)
	{
		HandleMessageExtensions (msg);

		switch (msg.type ())
		{
		case QXmppMessage::Chat:
			return HandlePrivateMessage (msg, nick);
		case QXmppMessage::GroupChat:
		case QXmppMessage::Normal:
			return HandlePublicMessage (msg, nick);
		default:
			qWarning () << "unhandled message type" << msg.type ();
			break;
		}
	}

	void RoomHandler::HandlePrivateMessage (const QXmppMessage& msg, const QString& nick)
	{
		if (nick.isEmpty ())
		{
			qWarning () << "got a private message from an empty nick, ignoring";
			return;
		}

		const auto& entry = GetParticipantEntry (nick, false);
		if (msg.isAttentionRequested ())
			entry->HandleAttentionMessage (msg);

		if (msg.state ())
			entry->UpdateChatState (msg.state (), QString ());

		if (!msg.body ().isEmpty ())
			entry->HandleMessage (new GlooxMessage (msg, Account_->GetClientConnection ().get ()));
	}

	void RoomHandler::HandlePublicMessage (const QXmppMessage& msg, const QString& nick)
	{
		if (!msg.subject ().isEmpty ())
		{
			Subject_ = msg.subject ();
			CLEntry_->HandleSubjectChanged (Subject_);

			const auto& string = nick.isEmpty () ?
					msg.subject () :
					tr ("%1 changed subject to %2").arg (nick, msg.subject ());

			const auto message = new RoomPublicMessage (string,
					IMessage::Direction::In,
					CLEntry_,
					IMessage::Type::EventMessage,
					IMessage::SubType::RoomSubjectChange);
			CLEntry_->HandleMessage (message);
			return;
		}

		if (msg.body ().isEmpty ())
			return;

		if (!nick.isEmpty ())
		{
			const bool existed = Nick2Entry_.contains (nick);
			const auto& entry = GetParticipantEntry (nick, false);
			CLEntry_->HandleMessage (new RoomPublicMessage (msg, CLEntry_, entry));
			if (!existed)
				RemoveEntry (entry.get ());
		}
		else
		{
			const auto message = new RoomPublicMessage (msg.body (),
					IMessage::Direction::In,
					CLEntry_,
					IMessage::Type::EventMessage,
					IMessage::SubType::Other);
			CLEntry_->HandleMessage (message);
		}
	}

	QList<ICLEntry*> RoomHandler::GetParticipants () const
	{
		return Util::Map (Nick2Entry_, [] (const auto& ptr) -> ICLEntry* { return ptr.get (); });
	}

	QString RoomHandler::GetSubject () const
	{
		return Subject_;
	}

	void RoomHandler::Join ()
	{
		VoluntaryLeave_ = false;
		if (Room_->isJoined ())
			return;

		Room_->join ();
	}

	void RoomHandler::SetSubject (const QString& subj)
	{
		Room_->setSubject (subj);
	}

	void RoomHandler::Leave (const QString& msg)
	{
		SendLeave (msg);
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

	void RoomHandler::SetAffiliation (const RoomParticipantEntry& entry, QXmppMucItem::Affiliation newAff, const QString& reason)
	{
		QXmppMucItem item;
		item.setNick (entry.GetNick ());
		item.setReason (reason);
		item.setAffiliation (newAff);
		Account_->GetClientConnection ()->Update (item, Room_->jid ());
	}

	void RoomHandler::SetRole (const RoomParticipantEntry& entry, QXmppMucItem::Role newRole, const QString& reason)
	{
		QXmppMucItem item;
		item.setNick (entry.GetNick ());
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
		if (newNick.isEmpty ())
		{
			qWarning () << "ignoring renaming into an empty nick";
			return;
		}

		if (!Nick2Entry_.contains (newNick))
		{
			const auto& newEntry = GetParticipantEntry (newNick, false);
			newEntry->SetAffiliation (entry->GetAffiliation ());
			newEntry->SetRole (entry->GetRole ());
			emit Account_->GetAccountEmitter ().gotCLItems ({ newEntry.get () });
		}

		PendingNickChanges_ << newNick;

		const auto& otherEntry = Nick2Entry_.value (newNick);
		otherEntry->StealMessagesFrom (entry.get ());
		CLEntry_->MoveMessages (entry, otherEntry);

		MakeNickChangeMessage (nick, newNick);
		emit Account_->GetAccountEmitter ().removedCLItems ({ Nick2Entry_.value (nick).get () });
		Nick2Entry_.remove (nick);
	}

	RoomParticipantEntry_ptr RoomHandler::FindParticipantEntry (const QString& nick) const
	{
		return Nick2Entry_.value (nick);
	}

	RoomParticipantEntry_ptr RoomHandler::GetParticipantEntry (const QString& nick, bool announce)
	{
		if (nick.isEmpty ())
			return {};

		auto& entry = Nick2Entry_ [nick];

		if (!entry)
		{
			entry = std::make_shared<RoomParticipantEntry> (nick, this, Account_);
			// TODO this won't be needed once messages are QObject-less
			connect (entry.get (),
					&RoomParticipantEntry::chatTabClosed,
					entry.get (),
					[this, entry = entry.get ()]
					{
						if (entry->GetStatus (QString ()).State_ == SOffline)
							RemoveEntry (entry);
					},
					Qt::QueuedConnection);
			if (announce)
				emit Account_->GetAccountEmitter ().gotCLItems ({ entry.get () });
		}

		return entry;
	}

	void RoomHandler::handleParticipantAdded (const QString& jid)
	{
		const auto nick = ClientConnection::Split (jid).Resource_;
		if (nick.isEmpty ())
			return;

		const auto& pres = Room_->participantPresence (jid);
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
			emit Account_->GetAccountEmitter ().gotCLItems ({ entry.get () });

		using enum MucEvents::ParticipantJoinOrder;
		emit CLEntry_->GetMUCEntryEmitter ().participantJoined (*entry, Room_->isJoined () ? AfterUs : BeforeUs);
	}

	void RoomHandler::handleParticipantChanged (const QString& jid)
	{
		const auto nick = ClientConnection::Split (jid).Resource_;
		if (nick.isEmpty ())
			return;

		const auto& pres = Room_->participantPresence (jid);

		const auto& entry = GetParticipantEntry (nick);

		entry->HandlePresence (pres, QString ());

		const auto& item = pres.mucItem ();
		if (item.affiliation () != entry->GetAffiliation () ||
				item.role () != entry->GetRole ())
			HandlePermsChanged (nick, item.affiliation (), item.role (), item.reason ());
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
		Account_->GetClientConnection ()->GetClient ()->send (std::move (msg));
	}

	void RoomHandler::HandleSelfRemoved (const QXmppPresence& pres)
	{
		auto& emitter = CLEntry_->GetMUCEntryEmitter ();

		const auto banned = pres.mucStatusCodes ().contains (301);
		const auto kicked = pres.mucStatusCodes ().contains (307);
		const auto& reason = pres.mucItem ().reason ();

		if (banned)
		{
			RemoveThis ();
			emit emitter.beenBanned (reason);
		}
		else if (kicked)
		{
			RemoveThis ();
			emit emitter.beenKicked (reason);
		}
		else if (!VoluntaryLeave_)
		{
			qWarning () << "detected involuntary leave, rejoining" << RoomJID_;
			RemoveThis ();
			const auto& data = CLEntry_->GetIdentifyingData ();
			QTimer::singleShot (5000, Account_, [data, acc = Account_] { RejoinMuc (*acc, data); });
		}
	}

	void RoomHandler::HandleParticipantRemoved (const QString& nick, const QXmppPresence& pres)
	{
		const auto entry = FindParticipantEntry (nick);
		if (!entry)
			return;

		const auto banned = pres.mucStatusCodes ().contains (301);
		const auto kicked = pres.mucStatusCodes ().contains (307);
		const auto& reason = pres.mucItem ().reason ();

		using enum MucEvents::ParticipantForcedOut::Action;
		MucEvents::ParticipantLeaveInfo leaveInfo;
		if (banned)
			leaveInfo = MucEvents::ParticipantForcedOut { .Reason_ = reason, .Action_ = Banned };
		else if (kicked)
			leaveInfo = MucEvents::ParticipantForcedOut { .Reason_ = reason, .Action_ = Kicked };
		else
			leaveInfo = MucEvents::ParticipantLeft { pres.statusText () };
		emit CLEntry_->GetMUCEntryEmitter ().participantLeaving (*entry, leaveInfo);

		// TODO this will be unnecessary once messages become plain structs emitted from entries
		if (entry->HasUnreadMsgs ())
			entry->SetStatus (EntryStatus (SOffline, reason),
					QString (), QXmppPresence (QXmppPresence::Unavailable));
		else
			RemoveEntry (entry.get ());

		emit CLEntry_->GetMUCEntryEmitter ().participantLeft (*entry, leaveInfo);
	}

	bool RoomHandler::IsGateway () const
	{
		return IsGateway_.value_or (true);
	}

	void RoomHandler::RemoveEntry (RoomParticipantEntry *entry)
	{
		emit Account_->GetAccountEmitter ().removedCLItems ({ entry });
		Nick2Entry_.remove (entry->GetNick ());
	}

	void RoomHandler::SendLeave (const QString& reason)
	{
		VoluntaryLeave_ = true;
		Room_->leave (reason);
	}

	void RoomHandler::RemoveParticipants ()
	{
		if (const auto entries = GetParticipants ();
			!entries.isEmpty ())
			emit Account_->GetAccountEmitter ().removedCLItems (entries);

		Nick2Entry_.clear ();
	}

	void RoomHandler::RemoveThis ()
	{
		RemoveParticipants ();
		emit Account_->GetAccountEmitter ().removedCLItems ({ CLEntry_ });
		Account_->GetClientConnection ()->Unregister (this);

		Room_->deleteLater ();
		deleteLater ();
	}
}
}
}
