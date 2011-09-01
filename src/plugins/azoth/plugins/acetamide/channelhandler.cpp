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

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelHandler::ChannelHandler (IrcServerHandler *ish, const
			ChannelOptions& channel)
	: ChannelID_ (channel.ChannelName_ + "@" + channel.ServerName_)
	, ISH_ (ish)
	, ChannelOptions_ (channel)
	, IsRosterReceived_ (false)
	{
		ChannelCLEntry_ = new ChannelCLEntry (this);
		connect (this,
				SIGNAL (updateChanModes (const ChannelModes&)),
				ChannelCLEntry_,
				SIGNAL (gotNewChannelModes (const ChannelModes&)));
	}

	QString ChannelHandler::GetChannelID () const
	{
		return ChannelID_;
	}

	ChannelCLEntry* ChannelHandler::GetCLEntry () const
	{
		return ChannelCLEntry_;
	}

	IrcServerHandler* ChannelHandler::GetIrcServerHandler () const
	{
		return ISH_;
	}

	ChannelOptions ChannelHandler::GetChannelOptions () const
	{
		return ChannelOptions_;
	}

	QList<QObject*> ChannelHandler::GetParticipants () const
	{
		QList<QObject*> result;
		Q_FOREACH (ServerParticipantEntry_ptr spe,
				ISH_->GetParticipantsOnChannel (ChannelOptions_.ChannelName_))
			result << spe.get ();
		return result;
	}

	ServerParticipantEntry_ptr ChannelHandler::GetSelf ()
	{
		Q_FOREACH (ServerParticipantEntry_ptr spe,
				ISH_->GetParticipantsOnChannel (ChannelOptions_.ChannelName_))
		{
			if (spe->GetEntryName () == ISH_->GetNickName ())
				return spe;
		}

		return ServerParticipantEntry_ptr ();
	}

	IrcMessage* ChannelHandler::CreateMessage (IMessage::MessageType t,
			const QString& variant, const QString& body)
	{
		IrcMessage *msg = new IrcMessage (t,
				IMessage::DIn,
				variant,
				ISH_->GetNickName (),
				ISH_->GetAccount ()->GetClientConnection ().get ());
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());

		return msg;
	}

	bool ChannelHandler::IsRosterReceived () const
	{
		return IsRosterReceived_;
	}

	void ChannelHandler::SetRosterReceived (bool status)
	{
		IsRosterReceived_ = status;
	}

	void ChannelHandler::ShowServiceMessage (const QString& msg,
			IMessage::MessageType mt, IMessage::MessageSubType mst)
	{
		ChannelPublicMessage *message = new ChannelPublicMessage (msg,
				IMessage::DIn,
				ChannelCLEntry_,
				mt,
				mst);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SendPublicMessage (const QString& msg)
	{
		if (GetSelf () == ServerParticipantEntry_ptr ())
			return;
		bool command = false;
		if (msg.startsWith ('/'))
		{
			ISH_->ParseMessageForCommand (msg, ChannelID_);
			command = true;
			ShowServiceMessage (msg, IMessage::MTEventMessage, IMessage::MSTOther);
		}
		else
		{
			ISH_->SendPublicMessage (msg, ChannelID_);
			HandleIncomingMessage (ISH_->GetNickName (), msg);
		}
	}

	void ChannelHandler::HandleIncomingMessage (const QString& nick,
			const QString& msg)
	{
		ServerParticipantEntry_ptr entry =
				ISH_->GetParticipantEntry (nick);

		if (!entry)
			return;

		ChannelPublicMessage *message =
				new ChannelPublicMessage (msg,
						IMessage::DIn,
						ChannelCLEntry_,
						IMessage::MTMUCMessage,
						IMessage::MSTOther,
						entry);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetChannelUser (const QString& nick)
	{
		ServerParticipantEntry_ptr entry = ISH_->
				GetParticipantEntry (nick);
		QStringList groups = entry->GetChannels ();
		if (!groups.contains (ChannelOptions_.ChannelName_))
		{
			groups << ChannelOptions_.ChannelName_;
			entry->SetGroups (groups);
			MakeJoinMessage (nick);
			entry->SetStatus (EntryStatus (SOnline, QString ()));
		}
	}

	void ChannelHandler::MakeJoinMessage (const QString& nick)
	{
		QString msg  = tr ("%1 joined the channel").arg (nick);

		ChannelPublicMessage *message =
				new ChannelPublicMessage (msg,
					IMessage::DIn,
					ChannelCLEntry_,
					IMessage::MTStatusMessage,
					IMessage::MSTParticipantJoin);
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
					ChannelCLEntry_,
					IMessage::MTStatusMessage,
					IMessage::MSTParticipantLeave);

		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakeKickMessage (const QString& nick,
			const QString& msg, const QString& who)
	{
		QString mess;
		QString reason = QString ();
		if (!msg.isEmpty ())
			reason = ":" + msg;

		if (nick == ISH_->GetNickName ())
			mess = tr ("You have been kicked by %1 %2")
					.arg (who, reason);
		else if (who == ISH_->GetNickName ())
			mess = tr ("You kicked %1 %2")
					.arg (nick, reason);
		else
			mess = tr ("%1 has been kicked by %2 %3")
					.arg (nick, who, reason);

		ChannelPublicMessage *message = new ChannelPublicMessage (mess,
				IMessage::DIn,
				ChannelCLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTKickNotification);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetMUCSubject (const QString& subject)
	{
		Subject_ = subject;

		ChannelPublicMessage *message =
				new ChannelPublicMessage (subject,
							IMessage::DIn,
							ChannelCLEntry_,
							IMessage::MTEventMessage,
							IMessage::MSTRoomSubjectChange);
		ChannelCLEntry_->HandleMessage (message);
	}

	QString ChannelHandler::GetMUCSubject () const
	{
		return Subject_;
	}

	void ChannelHandler::Leave (const QString& msg)
	{
		ISH_->LeaveChannel (ChannelOptions_.ChannelName_, msg);
	}

	void ChannelHandler::CloseChannel ()
	{
		Q_FOREACH (ServerParticipantEntry_ptr entry,
				ISH_->GetParticipantsOnChannel (ChannelOptions_.ChannelName_))
		{
			QStringList list = entry->GetChannels ();
			bool prChat = entry->IsPrivateChat ();

			if (list.contains (ChannelOptions_.ChannelName_))
			{
				list.removeAll (ChannelOptions_.ChannelName_);
				if (!list.count () && !prChat)
				{
					ISH_->GetAccount ()->
							handleEntryRemoved (entry.get ());
					ISH_->RemoveParticipantEntry (entry->
							GetEntryName ());
				}
				else
					entry->SetGroups (list);
			}
		}

		RemoveThis ();
	}

	void ChannelHandler::LeaveParticipant (const QString& nick, 
			const QString& msg)
	{
		if (RemoveUserFromChannel (nick))
			MakeLeaveMessage (nick, msg);
	}

	void ChannelHandler::KickParticipant (const QString& nick, 
			const QString& target, const QString& msg)
	{
		if (RemoveUserFromChannel (target))
			MakeKickMessage (target, msg, nick);
	}

	void ChannelHandler::RemoveThis ()
	{
		ISH_->GetAccount ()->handleEntryRemoved (ChannelCLEntry_);

		ISH_->UnregisterChannel (this);

		deleteLater ();
	}

	void ChannelHandler::RequestBanList ()
	{
		ISH_->GetBanList (ChannelOptions_.ChannelName_);
	}

	void ChannelHandler::RequestExceptList ()
	{
		ISH_->GetExceptList (ChannelOptions_.ChannelName_);
	}

	void ChannelHandler::RequestInviteList ()
	{
		ISH_->GetInviteList (ChannelOptions_.ChannelName_);
	}

	void ChannelHandler::AddBanListItem (QString mask)
	{
		ISH_->AddBanListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveBanListItem (QString mask)
	{
		ISH_->RemoveBanListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::AddExceptListItem (QString mask)
	{
		ISH_->AddExceptListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveExceptListItem (QString mask)
	{
		ISH_->RemoveExceptListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::AddInviteListItem (QString mask)
	{
		ISH_->AddInviteListItem (ChannelOptions_.ChannelName_, mask);
	}

	void ChannelHandler::RemoveInviteListItem (QString mask)
	{
		ISH_->RemoveInviteListItem (ChannelOptions_.ChannelName_, mask);
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
			ShowServiceMessage (msg, IMessage::MTEventMessage, IMessage::MSTOther);
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
			ShowServiceMessage (msg, IMessage::MTEventMessage, IMessage::MSTOther);
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
			ShowServiceMessage (msg, IMessage::MTEventMessage, IMessage::MSTOther);
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
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
		ShowServiceMessage (msg,
				IMessage::MTEventMessage, IMessage::MSTOther);
		emit updateChanModes (ChannelMode_);
	}

	void ChannelHandler::SetNewChannelModes(const ChannelModes& modes)
	{
		ISH_->SetNewChannelModes (ChannelOptions_.ChannelName_, modes);
	}

	bool ChannelHandler::RemoveUserFromChannel (const QString& nick)
	{
		ServerParticipantEntry_ptr entry =
				ISH_->GetParticipantEntry (nick);
		if (!entry)
			return false;

		QStringList groups = entry->GetChannels ();
		if (groups.contains (ChannelOptions_.ChannelName_))
		{
			if (groups.removeOne (ChannelOptions_.ChannelName_))
			{
				ISH_->GetAccount ()->handleEntryRemoved (entry.get ());
				if (!groups.count () && !entry->IsPrivateChat ())
					ISH_->RemoveParticipantEntry (nick);
				else
					ISH_->GetParticipantEntry (nick)->SetGroups (groups);
			}
		}

		return true;
	}

};
};
};
