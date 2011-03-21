/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include <interfaces/iproxyobject.h>
#include "ircaccount.h"
#include "channelclentry.h"
#include "channelpublicmessage.h"
#include "clientconnection.h"
#include "ircmessage.h"


namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelHandler::ChannelHandler (const ServerOptions& server, 
			const ChannelOptions& channel, IrcAccount *account)
	: Account_ (account)
	, CLEntry_ (new ChannelCLEntry (this, Account_))
	, Channel_ (channel)
	, Server_ (server)
	, ChannelID_ (channel.ChannelName_ + "@" + channel.ServerName_)
	, ServerID_ (server.ServerName_ + ":" + QString::number (server.ServerPort_))
	, Nickname_ (server.ServerNicknames_.at (0))
	, Subject_ (QString ())
	{
	}
	
	QString ChannelHandler::GetChannelID () const
	{
		return ChannelID_;
	}

	ChannelCLEntry* ChannelHandler::GetCLEntry () const
	{
		return CLEntry_;
	}

	QList<QObject*> ChannelHandler::GetParticipants () const
	{
		QList<QObject*> result;
		Q_FOREACH (ServerParticipantEntry_ptr chpe, Nick2Entry_.values ())
			result << chpe.get ();
		return result;
	}

	IrcMessage* ChannelHandler::CreateMessage (IMessage::MessageType type,
			const QString& nick, const QString& body)
	{
		IrcMessage *message = new IrcMessage (IMessage::MTMUCMessage,
				IMessage::DOut,
				GetChannelID (),
				nick,
				Account_->GetClientConnection ().get ());
		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());
		return message;
	}

	QString ChannelHandler::GetNickname () const
	{
		return Nickname_;
	}
	
	void ChannelHandler::SetNickname (const QString& nick)
	{
		Nickname_ = nick;
	}

	QString ChannelHandler::GetSubject () const
	{
		return Subject_;
	}

	void ChannelHandler::SetSubject (const QString& subject)
	{
		Subject_ = subject;
	}

	void ChannelHandler::Leave (const QString& msg)
	{
// 		Q_FOREACH (ServerParticipantEntry_ptr entry, Nick2Entry_.values ())
// 		{
// 			QStringList list = entry->GetChannels ();
// 			if (list.contains (Channel_.ChannelName_))
// 			{
// 				list.removeOne (Channel_.ChannelName_);
// 				if (!list.count ())
// 				{
// 					Account_->handleEntryRemoved (entry.get ());
// 					Account_->GetClientConnection ()->
// 							RemoveEntry (ServerID_, entry->GetEntryName ());
// 				}
// 				else
// 				{
// 					entry->SetGroups (list);
// 					qDebug () << entry->GetChannels ();
// 				}
// 				if (entry->IsPrivateChat ())
// 				{
// 					if (!list.count () && !list.contains (ServerID_))
// 						list << ServerID_;
// 					entry->SetGroups (list);
// 				}
// 				else if (list.count ())
// 					entry->SetGroups (list);
// 				else
// 				{
// 					Account_->handleEntryRemoved (entry.get ());
// 					Account_->GetClientConnection ()->
// 							RemoveEntry (ServerID_, entry->GetEntryName ());
// 				}
// 			}
// 		}
// 		Core::Instance ().GetServerManager ()->
// 				LeaveChannel (Channel_.ChannelName_, ServerID_, Account_);
// 		RemoveThis ();
	}

	void ChannelHandler::UserLeave (const QString& nick, const QString& msg)
	{
		ServerParticipantEntry_ptr entry = Account_->GetClientConnection ()->
				GetServerParticipantEntry (ServerID_, nick);
		QStringList list = entry->GetChannels ();
		if (list.contains (Channel_.ChannelName_))
		{
			if (list.removeOne (Channel_.ChannelName_));
			{
				Account_->handleEntryRemoved (entry.get ());
				if (list.count () || entry->IsPrivateChat ())
					entry->SetGroups (list);
				MakeLeaveMessage (nick, msg);
				Nick2Entry_.remove (nick);
			}
		}
	}

	void ChannelHandler::SetChannelUser (const QString& nick)
	{
		const bool existed = Nick2Entry_.contains (nick);
		ServerParticipantEntry_ptr entry = Account_->GetClientConnection ()->
				GetServerParticipantEntry (ServerID_, nick);
		if (!existed)
		{
			QStringList list = entry->GetChannels ();
			qDebug () << 3 << list;
			if (!list.contains (Channel_.ChannelName_))
				list << Channel_.ChannelName_;
			qDebug () << 4 << list;
			entry->SetGroups (list);
			Nick2Entry_ [nick] = entry;
			MakeJoinMessage (nick);
			entry->SetStatus (EntryStatus (SOnline, QString ()));
		}
	}

	void ChannelHandler::MakeJoinMessage (const QString& nick)
	{
		QString msg  = tr ("%1 joined the channel").arg (nick);

		ChannelPublicMessage *message = new ChannelPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantJoin);
		CLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakeLeaveMessage (const QString& nick, const QString& leaveMsg)
	{
		QString msg;
		if (!leaveMsg.isEmpty ())
			msg = tr ("%1 has left the room (%2)")
					.arg (nick, leaveMsg);
		else
			msg = tr ("%1 has left the room")
					.arg (nick);
		
		ChannelPublicMessage *message = new ChannelPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantLeave);
		CLEntry_->HandleMessage (message);
	}

	void ChannelHandler::HandleMessage (const QString& msg, const QString& nick)
	{
		ServerParticipantEntry_ptr entry = Account_->GetClientConnection ()->
				GetServerParticipantEntry (ServerID_, nick, false);
		ChannelPublicMessage *message = 0;
		if (!nick.isEmpty ())
			message = new ChannelPublicMessage (msg, 
				IMessage::DIn,
				CLEntry_,
				IMessage::MTMUCMessage,
				IMessage::MSTOther,
				entry);

		if (message)
			CLEntry_->HandleMessage (message);
	}

	ChannelOptions ChannelHandler::GetChannelOptions () const
	{
		return Channel_;
	}

	ServerOptions ChannelHandler::GetServerOptions () const
	{
		return Server_;
	}

	ServerParticipantEntry_ptr ChannelHandler::GetParticipantEntry (const QString& nick) const
	{
		if (Nick2Entry_.contains (nick))
			return Nick2Entry_ [nick];
	}

	void ChannelHandler::RemoveThis ()
	{
		Nick2Entry_.clear ();

		Account_->handleEntryRemoved (CLEntry_);

		Account_->GetClientConnection ()->Unregister (this);

		deleteLater ();
	}
};
};
};
