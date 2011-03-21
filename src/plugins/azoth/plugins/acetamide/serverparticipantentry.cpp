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

#include "serverparticipantentry.h"
#include <QAction>
#include "clientconnection.h"
#include "ircaccount.h"
#include "ircmessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ServerParticipantEntry::ServerParticipantEntry (const QString& nick, 
			const QString& server, IrcAccount *acc)
	: Account_ (acc)
	, EntryBase (acc)
	, ServerKey_ (server)
	, NickName_ (nick)
	, PrivateChat_ (false)
	{
		QAction *action = new QAction (tr ("Quit chat"), Account_);

		connect (action,
				SIGNAL (triggered (bool)),
				this,
				SLOT (closePrivateChat (bool)));

		connect (this,
				SIGNAL (removeFromList (const QString&, const QString&)),
				Account_->GetClientConnection ().get (),
				SLOT (removeServerParticipantEntry (const QString&, const QString&)));

		Actions_ << action;
	}

	QObject* ServerParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	QObject* ServerParticipantEntry::GetParentCLEntry () const
	{
		return NULL;
	}

	ICLEntry::Features ServerParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType ServerParticipantEntry::GetEntryType () const
	{
		return ETChat;
	}

	QString ServerParticipantEntry::GetEntryName () const
	{
		return NickName_;
	}

	void ServerParticipantEntry::SetEntryName (const QString& nick)
	{
		NickName_ = nick;
		emit nameChanged (NickName_);
	}

	QString ServerParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ServerKey_ + "_" + NickName_;
	}

	QString ServerParticipantEntry::GetHumanReadableID () const
	{
		return ServerKey_ + "_" + NickName_;
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
		emit groupsChanged (Channels_);
	}

	QStringList ServerParticipantEntry::Variants () const
	{
		return QStringList (QString ());
	}

	QObject* ServerParticipantEntry::CreateMessage (IMessage::MessageType type, 
			const QString& , const QString& body)
	{
		IrcMessage *message = new IrcMessage (IMessage::MTMUCMessage,
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

	Role ServerParticipantEntry::GetRole () const
	{
		return Role_;
	}

	void ServerParticipantEntry::SetRole (const Role& role)
	{
		Role_ = role;
	}

	Affilation ServerParticipantEntry::GetAffilation () const
	{
		return Affilation_;
	}

	void ServerParticipantEntry::SetAffialtion (const Affilation& aff)
	{
		Affilation_ = aff;
	}

	void ServerParticipantEntry::closePrivateChat (bool)
	{
		if (PrivateChat_ && (Channels_.count () == 1) && Channels_.first () == ServerKey_)
		{
			emit removeFromList (ServerKey_, NickName_);
		}
	}

};
};
};
