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
				ISH_->GetParticipants (ChannelOptions_.ChannelName_))
			result << spe.get ();
		return result;
	}

	ServerParticipantEntry_ptr ChannelHandler::GetSelf ()
	{
		Q_FOREACH (ServerParticipantEntry_ptr spe,
				ISH_->GetParticipants (ChannelOptions_.ChannelName_))
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
		if (msg.startsWith ('/'))
		{
			ISH_->ParseMessageForCommand (msg, ChannelID_);
			return;
		}
		else
			ISH_->SendPublicMessage (msg, ChannelID_);

		ServerParticipantEntry_ptr entry =
				ISH_->GetParticipantEntry (ISH_->GetNickName ());

		if (!entry)
			return;

		ChannelPublicMessage *message = new ChannelPublicMessage (msg,
				IMessage::DIn,
				ChannelCLEntry_,
				IMessage::MTMUCMessage,
				IMessage::MSTOther,
				entry);
		ChannelCLEntry_->HandleMessage (message);
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
		QString nickname = nick;
		ChannelRole role;
		switch (nick [0].toAscii ())
		{
			case '~':
				role = Owner;
				break;
			case '&':
				role = Admin;
				break;
			case '@':
				role = Operator;
				break;
			case '%':
				role = HalfOperator;
				break;
			case '+':
				role = Voiced;
				break;
			default:
				role = Participant;
		}

		if (role != Participant)
			nickname = nickname.mid (1);

		ServerParticipantEntry_ptr entry = ISH_->
				GetParticipantEntry (nickname);
		QStringList groups = entry->GetChannels ();
		if (!groups.contains (ChannelOptions_.ChannelName_))
		{
			groups << ChannelOptions_.ChannelName_;
			entry->SetGroups (groups);
			entry->SetRole (ChannelOptions_.ChannelName_, role);
			MakeJoinMessage (nickname);
			entry->SetStatus (EntryStatus (SOnline, QString ()));
		}
	}

	void ChannelHandler::RemoveChannelUser (const QString& nick,
			const QString& msg, int type, const QString& who)
	{
		ServerParticipantEntry_ptr entry =
				ISH_->GetParticipantEntry (nick);
		if (!entry)
			return;

		QStringList groups = entry->GetChannels ();
		if (groups.contains (ChannelOptions_.ChannelName_))
		{
			if (groups.removeOne (ChannelOptions_.ChannelName_))
			{
				switch (type)
				{
				case 0:
					MakeLeaveMessage (nick, msg);
					break;
				case 1:
					MakeKickMessage (nick, msg, who);
					break;
// 				case 2:
// 					MakeBanMessage (nick, msg);
// 					break;
				}
				ISH_->GetAccount ()->handleEntryRemoved (entry.get ());
				if (!groups.count () && !entry->IsPrivateChat ())
					ISH_->RemoveParticipantEntry (nick);
				else
					ISH_->GetParticipantEntry (nick)->SetGroups (groups);
			}
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

	void ChannelHandler::LeaveChannel (const QString& msg, bool cmd)
	{
		if (cmd)
		{
			ISH_->LeaveChannel (ChannelOptions_.ChannelName_, msg);
			return;
		}

		Q_FOREACH (ServerParticipantEntry_ptr entry,
				ISH_->GetParticipants (ChannelOptions_.ChannelName_))
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

	void ChannelHandler::RemoveThis ()
	{
		ISH_->GetAccount ()->handleEntryRemoved (ChannelCLEntry_);

		ISH_->UnregisterChannel (this);

		deleteLater ();
	}

};
};
};
