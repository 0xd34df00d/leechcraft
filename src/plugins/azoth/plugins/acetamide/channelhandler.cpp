/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#include "channelhandler.h"
#include "channelclentry.h"
#include "channelpublicmessage.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircserverhandler.h"
#include "channelsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelHandler::ChannelHandler (const ChannelOptions& channel,
			ChannelsManager *cm)
	: CM_ (cm)
	, ChannelID_ (channel.ChannelName_ + "@" + channel.ServerName_)
	, ChannelOptions_ (channel)
	, IsRosterReceived_ (false)
	{
		ChannelCLEntry_.reset (new ChannelCLEntry (this));
		connect (this,
				SIGNAL (updateChanModes (const ChannelModes&)),
				ChannelCLEntry_.get (),
				SIGNAL (gotNewChannelModes (const ChannelModes&)));
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
		Q_FOREACH (ChannelParticipantEntry_ptr cpe, Nick2Entry_.values ())
			result << cpe.get ();
		return result;
	}

	ChannelParticipantEntry_ptr ChannelHandler::GetSelf ()
	{
		Q_FOREACH (ChannelParticipantEntry_ptr cpe, Nick2Entry_.values ())
			if (cpe->GetEntryName () == CM_->GetOurNick ())
				return cpe;

		return GetParticipantEntry (CM_->GetOurNick ());
	}

	ChannelParticipantEntry_ptr ChannelHandler::GetParticipantEntry (const QString& nick)
	{
		if (Nick2Entry_.contains (nick))
			return Nick2Entry_ [nick];

		ChannelParticipantEntry_ptr entry (CreateParticipantEntry (nick));
		Nick2Entry_ [nick] = entry;
		return entry;
	}

	bool ChannelHandler::IsUserExists (const QString& nick) const
	{
		return Nick2Entry_.contains (nick);
	}

	IrcMessage* ChannelHandler::CreateMessage (IMessage::MessageType t,
			const QString& variant, const QString& body)
	{
		IrcMessage *msg = new IrcMessage (t,
				IMessage::DIn,
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
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantNickChange,
				Nick2Entry_ [oldNick]);

		CM_->GetAccount ()->handleEntryRemoved (Nick2Entry_ [oldNick].get ());
		QList<ChannelRole> roles = Nick2Entry_ [oldNick]->Roles ();
		ChannelParticipantEntry_ptr entry = Nick2Entry_.take (oldNick);
		entry->SetEntryName (newNick);
		entry->SetRoles (roles);
		CM_->GetAccount ()->handleGotRosterItems (QObjectList () << entry.get ());

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
			IMessage::MessageType mt, IMessage::MessageSubType mst,
			ChannelParticipantEntry_ptr entry)
	{
		ChannelPublicMessage *message = new ChannelPublicMessage (msg,
				IMessage::DIn,
				ChannelCLEntry_.get (),
				mt,
				mst,
				entry);
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
		ChannelParticipantEntry_ptr entry = GetParticipantEntry (nick);

		ChannelPublicMessage *message =
				new ChannelPublicMessage (msg,
						IMessage::DIn,
						ChannelCLEntry_.get (),
						IMessage::MTMUCMessage,
						IMessage::MSTOther,
						entry);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetChannelUser (const QString& nick,
			const QString& user, const QString& host)
	{
		QString nickName = nick;
		bool hasRole = false;
		QChar roleSign;

		if (CM_->GetISupport ().contains ("PREFIX"))
		{
			const QStringList& prefixList = CM_->GetISupport () ["PREFIX"].split (')');
			int id = prefixList.value (1).indexOf (nick [0]);
			if (id != -1)
			{
				hasRole = true;
				nickName = nickName.mid (1);
				roleSign = prefixList.at (0) [id + 1];
			}
		}

		CM_->ClosePrivateChat (nickName);
		ChannelParticipantEntry_ptr entry (GetParticipantEntry (nickName));
		entry->SetUserName (user);
		entry->SetHostName (host);

		ChannelRole role;
		if (hasRole)
			switch (roleSign.toAscii ())
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
		else
			role = ChannelRole::Participant;

		entry->SetRole (role);
		MakeJoinMessage (nickName);
		entry->SetStatus (EntryStatus (SOnline, QString ()));
	}

	void ChannelHandler::MakeJoinMessage (const QString& nick)
	{
		QString msg  = tr ("%1 joined the channel as %2")
				.arg (nick)
				.arg (ChannelCLEntry_->Role2String (Nick2Entry_ [nick]->HighestRole ()));

		ChannelPublicMessage *message =
				new ChannelPublicMessage (msg,
					IMessage::DIn,
					ChannelCLEntry_.get (),
					IMessage::MTStatusMessage,
					IMessage::MSTParticipantJoin,
					Nick2Entry_ [nick]);

		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakeLeaveMessage (const QString& nick,
			const QString& msg)
	{
		QString mess;
		if (!msg.isEmpty ())
			mess = tr ("%1 has left the channel (%2)").arg (nick, msg);
		else
			mess = tr ("%1 has left the channel").arg (nick);

		ChannelPublicMessage *message =
				new ChannelPublicMessage (mess,
					IMessage::DIn,
					ChannelCLEntry_.get (),
					IMessage::MTStatusMessage,
					IMessage::MSTParticipantLeave,
					Nick2Entry_ [nick]);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakeKickMessage (const QString& nick,
			const QString& msg, const QString& who)
	{
		QString mess;
		QString reason = QString ();
		if (!msg.isEmpty ())
			reason = ":" + msg;

		const QString& ourNick = CM_->GetOurNick ();

		if (nick == ourNick)
			mess = tr ("You have been kicked by %1 %2")
					.arg (who, reason);
		else if (who == ourNick)
			mess = tr ("You kicked %1: %2")
					.arg (nick, reason);
		else
			mess = tr ("%1 has been kicked by %2: %3")
					.arg (nick, who, reason);

		ChannelPublicMessage *message = new ChannelPublicMessage (mess,
				IMessage::DIn,
				ChannelCLEntry_.get (),
				IMessage::MTEventMessage,
				IMessage::MSTKickNotification);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakePermsChangedMessage (const QString& nick,
			ChannelRole role, bool isSet)
	{
		const QString& roleStr = ChannelCLEntry_->Role2String (role);
		QString msg = isSet ?
				tr ("%1 is now %2").arg (nick, roleStr) :
				tr ("%1 is not %2 anymore").arg (nick, roleStr);

		ChannelPublicMessage *message = new ChannelPublicMessage (msg,
				IMessage::DIn,
				ChannelCLEntry_.get (),
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantRoleAffiliationChange,
				GetParticipantEntry (nick));
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetMUCSubject (const QString& subject)
	{
		if (Subject_ == subject)
			return;

		Subject_ = subject;

		QString subj ("Topic changed to: %1");
		ChannelPublicMessage *message =
				new ChannelPublicMessage (subj.arg (subject),
						IMessage::DIn,
						ChannelCLEntry_.get (),
						IMessage::MTEventMessage,
						IMessage::MSTRoomSubjectChange);
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

	void ChannelHandler::KickParticipant (const QString& nick,
			const QString& target, const QString& msg)
	{
		if (Nick2Entry_.contains (target))
			MakeKickMessage (target, msg, nick);

		RemoveUserFromChannel (target);
	}

	void ChannelHandler::SetRole (ChannelParticipantEntry *entry,
			const ChannelRole& role, const QString&)
	{
		QString mode;

		switch (role)
		{
		case ChannelRole::Participant:
			break;
		case ChannelRole::Voiced:
			if (entry->Roles ().contains (Voiced))
				mode = "-v";
			else
				mode = "+v";
			break;
		case ChannelRole::HalfOperator:
			if (entry->Roles ().contains (Voiced))
				mode = "-h";
			else
				mode = "+h";
			break;
		case ChannelRole::Operator:
			if (entry->Roles ().contains (Voiced))
				mode = "-o";
			else
				mode = "+o";
			break;
		case ChannelRole::Admin:
			if (entry->Roles ().contains (Voiced))
				mode = "-a";
			else
				mode = "+a";
			break;
		case ChannelRole::Owner:
			if (entry->Roles ().contains (Voiced))
				mode = "-q";
			else
				mode = "+q";
			break;
		}

		if (!mode.isEmpty ())
			CM_->SetNewChannelMode (ChannelOptions_.ChannelName_,
					mode, entry->GetEntryName ());
	}

	void ChannelHandler::ManageWithParticipant (ChannelParticipantEntry *entry,
			const ChannelManagment& manage)
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
					entry->GetEntryName (), "");
			break;
		case ChannelManagment::KickAndBan:
			AddBanListItem (entry->GetEntryName () + "!*@*");
			CM_->KickCommand (ChannelOptions_.ChannelName_,
					entry->GetEntryName (), "");
			break;
		}
	}

	void ChannelHandler::RemoveThis ()
	{
		Q_FOREACH (ChannelParticipantEntry_ptr entry, Nick2Entry_.values ())
		{
			const bool isPrivate = entry->IsPrivateChat ();
			const QString nick = entry->GetEntryName ();
			const auto& participants = CM_->GetParticipantsByNick (nick);

			if (participants.count () == 1)
			{
				CM_->GetAccount ()->handleEntryRemoved (entry.get ());
				if (isPrivate)
					CM_->CreateServerParticipantEntry (nick);
			}
			else
			{
				CM_->GetAccount ()->handleEntryRemoved (entry.get ());
				Nick2Entry_.remove (nick);
				Q_FOREACH (QObject *entryObj, participants.values ())
				{
					if (participants.key (entryObj) == ChannelOptions_.ChannelName_)
						continue;

					CM_->GetChannelHandler (participants.key (entryObj))->RemoveUserFromChannel (nick);
					CM_->GetChannelHandler (participants.key (entryObj))->
							GetParticipantEntry (nick)->SetStatus (EntryStatus (SOnline, ""));
				}
			}
		}
		Nick2Entry_.clear ();

		CM_->GetAccount ()->handleEntryRemoved (ChannelCLEntry_.get ());

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

	void ChannelHandler::AddBanListItem (QString mask)
	{
		CM_->AddBanListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveBanListItem (QString mask)
	{
		CM_->RemoveBanListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::AddExceptListItem (QString mask)
	{
		CM_->AddExceptListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveExceptListItem (QString mask)
	{
		CM_->RemoveExceptListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::AddInviteListItem (QString mask)
	{
		CM_->AddInviteListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveInviteListItem (QString mask)
	{
		CM_->RemoveInviteListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::SetBanListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		ChannelCLEntry_->SetBanListItem (mask, nick, date);
		if (!ChannelCLEntry_->GetIsWidgetRequest ())
		{
			const QString msg = tr ("%1 set by %2 on %3")
					.arg (mask)
					.arg (nick)
					.arg (date.toString ("dd.MM.yyyy hh:mm:ss"));
			HandleServiceMessage (msg, IMessage::MTEventMessage, IMessage::MSTOther);
		}
	}

	void ChannelHandler::SetExceptListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		ChannelCLEntry_->SetExceptListItem (mask, nick, date);
		if (!ChannelCLEntry_->GetIsWidgetRequest ())
		{
			const QString msg = tr ("%1 set by %2 on %3")
					.arg (mask)
					.arg (nick)
					.arg (date.toString ("dd.MM.yyyy hh:mm:ss"));
			HandleServiceMessage (msg, IMessage::MTEventMessage, IMessage::MSTOther);
		}
	}

	void ChannelHandler::SetInviteListItem (const QString& mask,
			const QString& nick, const QDateTime& date)
	{
		ChannelCLEntry_->SetInviteListItem (mask, nick, date);
		if (!ChannelCLEntry_->GetIsWidgetRequest ())
		{
			const QString msg = tr ("%1 set by %2 on %3")
					.arg (mask)
					.arg (nick)
					.arg (date.toString ("dd.MM.yyyy hh:mm:ss"));
			HandleServiceMessage (msg, IMessage::MTEventMessage, IMessage::MSTOther);
		}
	}

	ChannelModes ChannelHandler::GetChannelModes () const
	{
		return ChannelMode_;
	}

	void ChannelHandler::SetInviteMode (bool invite)
	{
		ChannelMode_.InviteMode_ = invite;
		QString msg;
		if (invite)
			msg = tr ("Channel mode set to invite only channel (+i)");
		else
			msg = tr ("Channel mode set to non invite only channel (-i)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetModerateMode (bool moderate)
	{
		ChannelMode_.ModerateMode_ = moderate;
		QString msg;
		if (moderate)
			msg = tr ("Channel mode set to moderate channel (+m)");
		else
			msg = tr ("Channel mode set to unmoderate channel (-m)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetBlockOutsideMessagesMode (bool block)
	{
		ChannelMode_.BlockOutsideMessageMode_ = block;
		QString msg;
		if (block)
			msg = tr ("Channel mode set to block outside messages (+n)");
		else
			msg = tr ("Channel mode set to not block outside messages (-n)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetPrivateMode (bool priv)
	{
		ChannelMode_.PrivateMode_ = priv;
		QString msg;
		if (priv)
			msg = tr ("Channel mode set to private channel (+p)");
		else
			msg = tr ("Channel mode set to non private channel (-p)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetSecretMode (bool secret)
	{
		ChannelMode_.SecretMode_ = secret;
		QString msg;
		if (secret)
			msg = tr ("Channel mode set to secret channel (+s)");
		else
			msg = tr ("Channel mode set to non secret channel (-s)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetServerReOpMode (bool reop)
	{
		ChannelMode_.ReOpMode_ = reop;
		QString msg;
		if (reop)
			msg = tr ("Reop flag is set (+r)");
		else
			msg = tr ("Reop flag is remove (-r)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetOnlyOpTopicChangeMode (bool topic)
	{
		ChannelMode_.OnlyOpChangeTopicMode_ = topic;
		QString msg;
		if (topic)
			msg = tr ("Change topic available only for channel operators (+t)");
		else
			msg = tr ("Change topic available not only for channel operators (-t)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetUserLimit (bool islimit, int limit)
	{
		ChannelMode_.UserLimit_ = qMakePair (islimit, limit);
		QString msg;
		if (islimit)
			msg = tr ("Limit user set to %1 (+l)").arg (limit);
		else
			msg = tr ("Remove limit user (-l)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetChannelKey (bool iskey, const QString& key)
	{
		ChannelMode_.ChannelKey_ = qMakePair (iskey, key);
		QString msg;
		if (iskey)
			msg = tr ("Channel key set to %1 (+k)").arg (key);
		else
			msg = tr ("Remove channel key (-k)");
		HandleServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
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

	bool ChannelHandler::RemoveUserFromChannel (const QString& nick)
	{
		if (!Nick2Entry_.contains (nick))
			return false;

		ChannelParticipantEntry_ptr entry = Nick2Entry_ [nick];
		Nick2Entry_.remove (nick);
		CM_->GetAccount ()->handleEntryRemoved (entry.get ());

		return true;
	}

	ChannelParticipantEntry_ptr ChannelHandler::CreateParticipantEntry (const QString& nick)
	{
		ChannelParticipantEntry_ptr entry (new ChannelParticipantEntry (nick,
				this, CM_->GetAccount ()));
		CM_->GetAccount ()->handleGotRosterItems (QObjectList () << entry.get ());
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

};
};
};
