/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "serverparticipantentry.h"
#include <QAction>
#include "clientconnection.h"
#include "ircaccount.h"
#include "ircserverclentry.h"
#include "ircmessage.h"
#include "ircserverhandler.h"


namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ServerParticipantEntry::ServerParticipantEntry (const QString& nick,
			const QString& server, IrcAccount *acc)
	: EntryBase (acc)
	, NickName_ (nick)
	, ServerKey_ (server)
	, PrivateChat_ (false)
	, Account_ (acc)
	{
		QAction *closeChat = new QAction (tr ("Quit chat"), this);
		connect (closeChat,
				SIGNAL (triggered (bool)),
				this,
				SLOT (closePrivateChat (bool)));
		Actions_ << closeChat;
	}

	QObject* ServerParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	QObject* ServerParticipantEntry::GetParentCLEntry () const
	{
		return Account_->GetClientConnection ()->
				GetIrcServerHandler (ServerKey_)->GetCLEntry ();
	}

	ICLEntry::Features ServerParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType ServerParticipantEntry::GetEntryType () const
	{
		return ETPrivateChat;
	}

	QString ServerParticipantEntry::GetEntryName () const
	{
		return NickName_;
	}

	void ServerParticipantEntry::SetEntryName (const QString& nick)
	{
		NickName_ = nick;

		Q_FOREACH (QObject *message, AllMessages_)
		{
			IrcMessage *msg = qobject_cast<IrcMessage*> (message);
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "is not an object of IrcMessage"
						<< message;
				continue;
			}
			msg->SetOtherVariant (nick);
		}
	}

	QString ServerParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ServerKey_ + "_" + NickName_;
	}

	QString ServerParticipantEntry::GetHumanReadableID () const
	{
		return NickName_ + "_" + ServerKey_;
	}

	QStringList ServerParticipantEntry::Groups () const
	{
		QStringList list;
		QString server = ServerKey_.split (':').at (0);
		Q_FOREACH (QString channel, Channels_)
			list << channel + "@" + server;
		return list;
	}

	void ServerParticipantEntry::SetGroups (const QStringList& channel)
	{
		Channels_ = channel;
		emit groupsChanged (Groups ());
	}

	QStringList ServerParticipantEntry::Variants () const
	{
		return QStringList (QString ());
	}

	QObject*
			ServerParticipantEntry::CreateMessage (IMessage::MessageType,
					const QString&, const QString& body)
	{
 		IrcMessage *message = new IrcMessage (IMessage::MTChatMessage,
				IMessage::DOut,
				ServerKey_,
				NickName_,
				Account_->GetClientConnection ().get ());

		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());

		PrivateChat_ = true;

		AllMessages_ << message;

		return message;
	}

	QStringList ServerParticipantEntry::GetChannels () const
	{
		return Channels_;
	}

	void ServerParticipantEntry::SetPrivateChat (bool value)
	{
		PrivateChat_ = value;
	}

	bool ServerParticipantEntry::IsPrivateChat () const
	{
		return PrivateChat_;
	}

	ChannelRole ServerParticipantEntry::GetRole (const QString& ch) const
	{
		return Channel2Role_ [ch];
	}

	void ServerParticipantEntry::SetRole (const QString& ch,
										  ChannelRole r)
	{
		Channel2Role_ [ch] = r;
	}

	void ServerParticipantEntry::closePrivateChat (bool)
	{
		if (PrivateChat_)
		{
			PrivateChat_ = false;
			if (!Channels_.count ())
				Account_->GetClientConnection ()->
						ClosePrivateChat (ServerKey_, NickName_);
		}
	}

};
};
};
