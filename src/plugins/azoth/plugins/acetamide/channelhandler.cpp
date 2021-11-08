/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelhandler.h"
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include "channelclentry.h"
#include "channelpublicmessage.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircserverhandler.h"
#include "channelsmanager.h"

namespace LC::Azoth::Acetamide
{
	ChannelHandler::ChannelHandler (const ChannelOptions& channel,
			ChannelsManager *cm)
	: CM_ (cm)
	, ChannelID_ (channel.ChannelName_ + "@" + channel.ServerName_)
	, ChannelOptions_ (channel)
	{
		ChannelCLEntry_ = std::make_shared<ChannelCLEntry> (this);
		connect (this,
				&ChannelHandler::updateChanModes,
				ChannelCLEntry_.get (),
				&ChannelCLEntry::gotNewChannelModes);
	}

	QString ChannelHandler::GetChannelID () const
	{
		return ChannelID_;
	}

	ChannelCLEntry* ChannelHandler::GetCLEntry () const
	{
		return ChannelCLEntry_.get ();
	}

	ChannelsManager* ChannelHandler::GetChannelsManager () const
	{
		return CM_;
	}

	QString ChannelHandler::GetParentID () const
	{
		return CM_->GetServerID ();
	}

	ChannelOptions ChannelHandler::GetChannelOptions () const
	{
		return ChannelOptions_;
	}

	QList<QObject*> ChannelHandler::GetParticipants () const
	{
		QList<QObject*> result;
		for (const auto& cpe : Nick2Entry_)
			result << cpe.get ();
		return result;
	}

	ChannelParticipantEntry_ptr ChannelHandler::GetSelf ()
	{
		for (const auto& cpe : Nick2Entry_)
			if (cpe->GetEntryName () == CM_->GetOurNick ())
				return cpe;

		return GetParticipantEntry (CM_->GetOurNick ());
	}

	ChannelParticipantEntry_ptr ChannelHandler::GetParticipantEntry (const QString& nick, bool announce)
	{
		if (Nick2Entry_.contains (nick))
			return Nick2Entry_ [nick];

		auto entry = CreateParticipantEntry (nick, announce);
		Nick2Entry_ [nick] = entry;
		return entry;
	}

	ChannelParticipantEntry_ptr ChannelHandler::GetExistingParticipantEntry (const QString& nick) const
	{
		return Nick2Entry_.value (nick);
	}

	bool ChannelHandler::IsUserExists (const QString& nick) const
	{
		return Nick2Entry_.contains (nick);
	}

	IrcMessage* ChannelHandler::CreateMessage (IMessage::Type t,
			const QString& variant, const QString& body)
	{
		const auto msg = new IrcMessage (t,
				IMessage::Direction::In,
				variant,
				CM_->GetOurNick (),
				CM_->GetAccount ()->GetClientConnection ().get ());
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());

		return msg;
	}

	void ChannelHandler::ChangeNickname (const QString& oldNick, const QString& newNick)
	{
		const QString mess = tr ("%1 has changed nickname to %2")
				.arg (oldNick, newNick);

		HandleServiceMessage (mess,
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantNickChange,
				Nick2Entry_ [oldNick]);

		emit CM_->GetAccount ()->removedCLItems ({ Nick2Entry_ [oldNick].get () });
		QList<ChannelRole> roles = Nick2Entry_ [oldNick]->Roles ();
		ChannelParticipantEntry_ptr entry = Nick2Entry_.take (oldNick);
		entry->SetEntryName (newNick);
		entry->SetRoles (roles);
		emit CM_->GetAccount ()->gotCLItems ({ entry.get () });

		Nick2Entry_ [newNick] = entry;
	}

	bool ChannelHandler::IsRosterReceived () const
	{
		return IsRosterReceived_;
	}

	void ChannelHandler::SetRosterReceived (bool status)
	{
		IsRosterReceived_ = status;
	}

	void ChannelHandler::HandleServiceMessage (const QString& msg,
			IMessage::Type mt, IMessage::SubType mst,
			ChannelParticipantEntry_ptr entry)
	{
		const auto message = new ChannelPublicMessage (msg,
				IMessage::Direction::In,
				ChannelCLEntry_.get (),
				mt,
				mst,
				std::move (entry));
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SendPublicMessage (const QString& msg)
	{
		if (GetSelf () == ChannelParticipantEntry_ptr ())
			return;

		CM_->SendPublicMessage (ChannelOptions_.ChannelName_, msg);
	}

	void ChannelHandler::HandleIncomingMessage (const QString& nick,
			const QString& msg)
	{
		const auto& entry = GetParticipantEntry (nick);

		const auto message = new ChannelPublicMessage (msg,
				IMessage::Direction::In,
				ChannelCLEntry_.get (),
				IMessage::Type::MUCMessage,
				IMessage::SubType::Other,
				entry);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetChannelUser (const QString& nick,
			const QString& user, const QString& host)
	{
		QString nickName = nick;
		bool hasRole = false;
		QChar roleSign;

		const auto& prefixList = CM_->GetISupport ().value (Lits::PREFIX).split (')');
		const int id = prefixList.value (1).indexOf (nick [0]);
		if (id != -1)
		{
			hasRole = true;
			nickName = nickName.mid (1);
			roleSign = prefixList.at (0) [id + 1];
		}

		CM_->ClosePrivateChat (nickName);

		const auto existed = Nick2Entry_.contains (nickName);

		const auto& entry = GetParticipantEntry (nickName, false);
		entry->SetUserName (user);
		entry->SetHostName (host);

		ChannelRole role = ChannelRole::Participant;
		if (hasRole)
			switch (roleSign.toLatin1 ())
			{
				case 'v':
					role = ChannelRole::Voiced;
					break;
				case 'h':
					role = ChannelRole::HalfOperator;
					break;
				case 'o':
					role = ChannelRole::Operator;
					break;
				case 'a':
					role = ChannelRole::Admin;
					break;
				case 'q':
					role = ChannelRole::Owner;
					break;
				default:
					role = ChannelRole::Participant;
 			}

		entry->SetRole (role);
		entry->SetStatus (EntryStatus (SOnline, QString ()));

		if (!existed)
			emit CM_->GetAccount ()->gotCLItems ({ entry.get () });

		MakeJoinMessage (nickName);
	}

	void ChannelHandler::MakeJoinMessage (const QString& nick)
	{
		QString msg  = tr ("%1 joined the channel as %2")
				.arg (nick,
					  ChannelCLEntry_->Role2String (Nick2Entry_ [nick]->HighestRole ()));

		const auto message = new ChannelPublicMessage (msg,
				IMessage::Direction::In,
				ChannelCLEntry_.get (),
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantJoin,
				Nick2Entry_ [nick]);

		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakeLeaveMessage (const QString& nick,
			const QString& msg)
	{
		const auto& mess = msg.isEmpty () ?
				tr ("%1 has left the channel").arg (nick) :
				tr ("%1 has left the channel (%2)").arg (nick, msg);

		const auto message = new ChannelPublicMessage (mess,
				IMessage::Direction::In,
				ChannelCLEntry_.get (),
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantLeave,
				Nick2Entry_ [nick]);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakeKickMessage (const QString& nick,
			const QString& msg, const QString& kicker)
	{
		QString mess;
		QString reason;
		if (!msg.isEmpty ())
			reason = ":" + msg;

		const QString& ourNick = CM_->GetOurNick ();

		if (nick == ourNick)
			mess = tr ("You have been kicked by %1 %2")
					.arg (kicker, reason);
		else if (kicker == ourNick)
			mess = tr ("You kicked %1: %2")
					.arg (nick, reason);
		else
			mess = tr ("%1 has been kicked by %2: %3")
					.arg (nick, kicker, reason);

		const auto message = new ChannelPublicMessage (std::move (mess),
				IMessage::Direction::In,
				ChannelCLEntry_.get (),
				IMessage::Type::EventMessage,
				IMessage::SubType::KickNotification);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakePermsChangedMessage (const QString& nick,
			ChannelRole role, bool isSet)
	{
		const QString& roleStr = ChannelCLEntry_->Role2String (role);
		const auto& msg = isSet ?
				tr ("%1 is now %2").arg (nick, roleStr) :
				tr ("%1 is not %2 anymore").arg (nick, roleStr);

		const auto message = new ChannelPublicMessage (msg,
				IMessage::Direction::In,
				ChannelCLEntry_.get (),
				IMessage::Type::StatusMessage,
				IMessage::SubType::ParticipantRoleAffiliationChange,
				GetParticipantEntry (nick));
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetMUCSubject (const QString& subject)
	{
		if (Subject_ == subject)
			return;

		Subject_ = subject;
		if (!Url_.isEmpty ())
			Subject_.append ("\nURL: " + Url_);

		const auto message = new ChannelPublicMessage (tr ("Topic changed to: %1").arg (subject),
				IMessage::Direction::In,
				ChannelCLEntry_.get (),
				IMessage::Type::EventMessage,
				IMessage::SubType::RoomSubjectChange);
		ChannelCLEntry_->HandleMessage (message);
	}

	QString ChannelHandler::GetMUCSubject () const
	{
		return Subject_;
	}

	void ChannelHandler::SetTopic (const QString& topic)
	{
		CM_->SetTopic (ChannelOptions_.ChannelName_.toLower (), topic);
	}

	void ChannelHandler::Leave (const QString& msg)
	{
		CM_->LeaveChannel (ChannelOptions_.ChannelName_.toLower (), msg);
		CloseChannel ();
	}

	void ChannelHandler::CloseChannel ()
	{
		RemoveThis ();
	}

	void ChannelHandler::LeaveParticipant (const QString& nick,
			const QString& msg)
	{
		if (Nick2Entry_.contains (nick))
			MakeLeaveMessage (nick, msg);
		RemoveUserFromChannel (nick);
	}

	void ChannelHandler::KickParticipant (const QString& kicker,
			const QString& kickee, const QString& msg)
	{
		if (Nick2Entry_.contains (kickee))
			MakeKickMessage (kickee, msg, kicker);

		RemoveUserFromChannel (kickee);
	}

	void ChannelHandler::SetRole (ChannelParticipantEntry *entry, ChannelRole role, const QString&)
	{
		QString mode;

		switch (role)
		{
		case ChannelRole::Participant:
			break;
		case ChannelRole::Voiced:
			mode = "v"_ql;
			break;
		case ChannelRole::HalfOperator:
			mode = "h"_ql;
			break;
		case ChannelRole::Operator:
			mode = "o"_ql;
			break;
		case ChannelRole::Admin:
			mode = "a"_ql;
			break;
		case ChannelRole::Owner:
			mode = "q"_ql;
			break;
		}

		if (!mode.isEmpty ())
			mode.prepend (entry->Roles ().contains (Voiced) ? '-' : '+');

		if (!mode.isEmpty ())
			CM_->SetNewChannelMode (ChannelOptions_.ChannelName_,
					mode, entry->GetEntryName ());
	}

	void ChannelHandler::ManageWithParticipant (ChannelParticipantEntry *entry, ChannelManagment manage)
	{
		switch (manage)
		{
		case ChannelManagment::BanByName:
			AddBanListItem (entry->GetEntryName () + "!*@*");
			break;
		case ChannelManagment::BanByUserAndDomain:
			AddBanListItem ("*!" + entry->GetUserName () + "@" + entry->GetHostName ());
			break;
		case ChannelManagment::BanByDomain:
			AddBanListItem ("*!*@" + entry->GetHostName ());
			break;
		case ChannelManagment::Kick:
			CM_->KickCommand (ChannelOptions_.ChannelName_,
					entry->GetEntryName (), {});
			break;
		case ChannelManagment::KickAndBan:
			AddBanListItem (entry->GetEntryName () + "!*@*");
			CM_->KickCommand (ChannelOptions_.ChannelName_,
					entry->GetEntryName (), {});
			break;
		}
	}

	void ChannelHandler::RemoveThis ()
	{
		const auto& entries = Util::Map (Nick2Entry_, [] (const auto& obj) -> QObject* { return obj.get (); });
		emit CM_->GetAccount ()->removedCLItems (entries);

		for (const auto& entry : Nick2Entry_)
			if (entry->IsPrivateChat ())
				CM_->CreateServerParticipantEntry (entry->GetEntryName ());

		Nick2Entry_.clear ();

		emit CM_->GetAccount ()->removedCLItems ({ ChannelCLEntry_.get () });

		CM_->UnregisterChannel (this);
	}

	void ChannelHandler::RequestBanList ()
	{
		CM_->RequestBanList (ChannelOptions_.ChannelName_);
	}

	void ChannelHandler::RequestExceptList ()
	{
		CM_->RequestExceptList (ChannelOptions_.ChannelName_);
	}

	void ChannelHandler::RequestInviteList ()
	{
		CM_->RequestInviteList (ChannelOptions_.ChannelName_);
	}

	void ChannelHandler::AddBanListItem (const QString& mask)
	{
		CM_->AddBanListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveBanListItem (const QString& mask)
	{
		CM_->RemoveBanListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::AddExceptListItem (const QString& mask)
	{
		CM_->AddExceptListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveExceptListItem (const QString& mask)
	{
		CM_->RemoveExceptListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::AddInviteListItem (const QString& mask)
	{
		CM_->AddInviteListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveInviteListItem (const QString& mask)
	{
		CM_->RemoveInviteListItem (ChannelOptions_.ChannelName_, mask);
	}

	namespace
	{
		void AddMaskMessage (ChannelHandler& handler, const QString& mask, const QString& nick, const QDateTime& date)
		{
			const auto& msg = ChannelHandler::tr ("%1 set by %2 on %3")
					.arg (mask,
						  nick,
						  date.toString (QStringLiteral ("dd.MM.yyyy hh:mm:ss")));
			handler.HandleServiceMessage (msg, IMessage::Type::EventMessage, IMessage::SubType::Other);
		}
	}

	void ChannelHandler::SetBanListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		ChannelCLEntry_->SetBanListItem (mask, nick, date);
		AddMaskMessage (*this, mask, nick, date);
	}

	void ChannelHandler::SetExceptListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		ChannelCLEntry_->SetExceptListItem (mask, nick, date);
		AddMaskMessage (*this, mask, nick, date);
	}

	void ChannelHandler::SetInviteListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		ChannelCLEntry_->SetInviteListItem (mask, nick, date);
		AddMaskMessage (*this, mask, nick, date);
	}

	ChannelModes ChannelHandler::GetChannelModes () const
	{
		return ChannelMode_;
	}

	void ChannelHandler::SetInviteMode (bool invite)
	{
		ChannelMode_.InviteMode_ = invite;
		const auto& msg = invite ?
				tr ("Channel mode set to invite only channel (+i)") :
				tr ("Channel mode set to non invite only channel (-i)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetModerateMode (bool moderate)
	{
		ChannelMode_.ModerateMode_ = moderate;
		const auto& msg = moderate ?
				tr ("Channel mode set to moderate channel (+m)") :
				tr ("Channel mode set to unmoderate channel (-m)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetBlockOutsideMessagesMode (bool block)
	{
		ChannelMode_.BlockOutsideMessageMode_ = block;
		const auto& msg = block ?
				tr ("Channel mode set to block outside messages (+n)") :
				tr ("Channel mode set to not block outside messages (-n)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetPrivateMode (bool priv)
	{
		ChannelMode_.PrivateMode_ = priv;
		const auto& msg = priv ?
				tr ("Channel mode set to private channel (+p)") :
				tr ("Channel mode set to non private channel (-p)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetSecretMode (bool secret)
	{
		ChannelMode_.SecretMode_ = secret;
		const auto& msg = secret ?
				tr ("Channel mode set to secret channel (+s)") :
				tr ("Channel mode set to non secret channel (-s)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetServerReOpMode (bool reop)
	{
		ChannelMode_.ReOpMode_ = reop;
		const auto& msg = reop ?
				tr ("Reop flag is set (+r)") :
				tr ("Reop flag is remove (-r)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetOnlyOpTopicChangeMode (bool topic)
	{
		ChannelMode_.OnlyOpChangeTopicMode_ = topic;
		const auto& msg = topic ?
				tr ("Change topic available only for channel operators (+t)") :
				tr ("Change topic available not only for channel operators (-t)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetUserLimit (bool islimit, int limit)
	{
		ChannelMode_.UserLimit_ = qMakePair (islimit, limit);
		const auto& msg = islimit ?
				tr ("Limit user set to %1 (+l)").arg (limit) :
				tr ("Remove limit user (-l)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetChannelKey (bool iskey, const QString& key)
	{
		ChannelMode_.ChannelKey_ = qMakePair (iskey, key);
		const auto& msg = iskey ?
				tr ("Channel key set to %1 (+k)").arg (key) :
				tr ("Remove channel key (-k)");
		HandleServiceMessage (msg,
				IMessage::Type::EventMessage, IMessage::SubType::Other);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetNewChannelModes (const ChannelModes& modes)
	{
		CM_->SetNewChannelModes (ChannelOptions_.ChannelName_, modes);
	}

	void ChannelHandler::UpdateEntry (const WhoMessage& message)
	{
		if (Nick2Entry_.contains (message.Nick_))
		{
			ChannelParticipantEntry_ptr entry = Nick2Entry_ [message.Nick_];
			entry->SetUserName (message.UserName_);
			entry->SetHostName (message.Host_);
			entry->SetRealName (message.RealName_);
			entry->SetStatus (message.IsAway_ ?
					EntryStatus (SAway, QString ()) :
					EntryStatus (SOnline, QString ()));
		}
	}

	void ChannelHandler::SetUrl (const QString& url)
	{
		Url_ = url;
		if (!Url_.isEmpty ())
			Subject_.append ("\nURL: " + Url_);
	}

	bool ChannelHandler::RemoveUserFromChannel (const QString& nick)
	{
		if (!Nick2Entry_.contains (nick))
			return false;

		ChannelParticipantEntry_ptr entry = Nick2Entry_ [nick];
		Nick2Entry_.remove (nick);
		emit CM_->GetAccount ()->removedCLItems ({ entry.get () });

		return true;
	}

	ChannelParticipantEntry_ptr ChannelHandler::CreateParticipantEntry (const QString& nick, bool announce)
	{
		auto entry = std::make_shared<ChannelParticipantEntry> (nick, this, CM_->GetAccount ());
		if (announce)
			emit CM_->GetAccount ()->gotCLItems ({ entry.get () });
		return entry;
	}

	void ChannelHandler::handleWhoIs (const QString& nick)
	{
		CM_->RequestWhoIs (ChannelOptions_.ChannelName_.toLower (), nick);
	}

	void ChannelHandler::handleWhoWas (const QString& nick)
	{
		CM_->RequestWhoWas (ChannelOptions_.ChannelName_.toLower (), nick);
	}

	void ChannelHandler::handleWho (const QString& nick)
	{
		CM_->RequestWho (ChannelOptions_.ChannelName_.toLower (), nick);
	}

	void ChannelHandler::handleCTCPRequest (const QStringList& cmd)
	{
		CM_->CTCPRequest (cmd, ChannelOptions_.ChannelName_.toLower ());
	}
}
