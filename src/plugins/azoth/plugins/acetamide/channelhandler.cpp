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
#include "channelparticipantentry.h"
#include "clientconnection.h"
#include "ircprotocol.h"
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
	, ChannelID_ (QString ("%1@%2").arg (channel.ChannelName_, channel.ServerName_))
	, Nickname_ (server.ServerNicknames_.at (0))
	, Subject_ (QString ())
	, Channel_ (channel)
	, Server_ (server)
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
		Q_FOREACH (ChannelParticipantEntry_ptr rpe, Nick2Entry_.values ())
			result << rpe.get ();
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
	
	QString ChannelHandler::SetNickname (const QString& nick)
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
		Q_FOREACH (ChannelParticipantEntry_ptr entry, Nick2Entry_.values ())
			Account_->handleEntryRemoved (entry.get ());

		QString serverId = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		
		Core::Instance ().GetServerManager ()->
				LeaveChannel (Channel_.ChannelName_, Account_);
		RemoveThis ();
	}

	void ChannelHandler::UserLeave (const QString& nick)
	{
		ChannelParticipantEntry_ptr entry = GetParticipantEntry (nick);
		MakeLeaveMessage (nick);
		Account_->handleEntryRemoved (entry.get ());
		Nick2Entry_.remove (nick);
	}

	void ChannelHandler::SetChannelUser (const QString& nick)
	{
		const bool existed = Nick2Entry_.contains (nick);
		ChannelParticipantEntry_ptr entry = GetParticipantEntry (nick);

		if (!existed)
			MakeJoinMessage (nick);
// 		else
// 			MakeStatusChangedMessage (pres, nick);

// 		if (pres.type () == QXmppPresence::Unavailable)
// 		{
// 			MakeLeaveMessage (pres, nick);
// 
// 			Account_->handleEntryRemoved (entry.get ());
// 			Nick2Entry_.remove (nick);
// 			return;
// 		}
// 
// 		entry->SetClientInfo ("", pres);
// 
// 		const QXmppPresence::Status& xmppSt = pres.status ();
// 		EntryStatus status (static_cast<State> (xmppSt.type ()),
// 				xmppSt.statusText ());
// 		entry->SetStatus (status, QString ());
// 
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

	void ChannelHandler::MakeLeaveMessage (const QString& nick)
	{
		QString msg = tr ("%1 has left the room").arg (nick);
		
		ChannelPublicMessage *message = new ChannelPublicMessage (msg,
				IMessage::DIn,
				CLEntry_,
				IMessage::MTStatusMessage,
				IMessage::MSTParticipantLeave);
		CLEntry_->HandleMessage (message);
	}

	void ChannelHandler::HandleMessage (const QString& msg, const QString& nick)
	{
		ChannelParticipantEntry_ptr entry = GetParticipantEntry (nick, false);
// 		if (!nick.isEmpty ())
// 		{
// 			IrcMessage *message = new IrcMessage (msg,
// 					ChannelID_, Account_->GetClientConnection ().get ());
// 			entry->HandleMessage (message);
// 		}
// 		else
// 		{
		ChannelPublicMessage *message = 0;
		if (!nick.isEmpty ())
			message = new ChannelPublicMessage (msg, 
				IMessage::DIn,
				CLEntry_,
				IMessage::MTMUCMessage,
				IMessage::MSTOther,
				entry);
// 		else
// 			message = new ChannelPublicMessage (msg,
// 				IMessage::DIn,
// 				CLEntry_,
// 				IMessage::MTEventMessage,
// 				IMessage::MSTOther);
		if (message)
			CLEntry_->HandleMessage (message);
// 	}
	}

	ChannelOptions ChannelHandler::GetChannelOptions () const
	{
		return Channel_;
	}

	ServerOptions ChannelHandler::GetServerOptions () const
	{
		return Server_;
	}

	ChannelParticipantEntry_ptr ChannelHandler::GetParticipantEntry (const QString& nick, bool announce)
	{
		if (!Nick2Entry_.contains (nick))
		{
			ChannelParticipantEntry_ptr entry (CreateParticipantEntry (nick, announce));
			Nick2Entry_ [nick] = entry;
			return entry;
		}
		else
			return Nick2Entry_ [nick];
	}

	ChannelParticipantEntry_ptr ChannelHandler::CreateParticipantEntry (const QString& nick, bool announce)
	{
		ChannelParticipantEntry_ptr entry (new ChannelParticipantEntry (nick,
					this, Account_));
		Nick2Entry_ [nick] = entry;
		if (announce)
			Account_->handleGotRosterItems (QList<QObject*> () << entry.get ());
		return entry;
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
