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
			IrcServerHandler *ish, IrcAccount *acc)
	: IrcParticipantEntry (nick, acc)
	, ISH_ (ish)
	{
		ServerID_ = ish->GetServerID ();
	}

	QObject* ServerParticipantEntry::GetParentCLEntry () const
	{
		return ISH_->GetCLEntry ();
	}

	QString ServerParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ISH_->GetServerID () + "_" + Nick_;
	}

	QString ServerParticipantEntry::GetHumanReadableID () const
	{
		return Nick_ + "_" + ISH_->GetServerID ();
	}

	QStringList ServerParticipantEntry::Groups () const
	{
		return QStringList (tr ("Private chats"));
	}

	void ServerParticipantEntry::SetGroups (const QStringList&)
	{
	}

	QObject* ServerParticipantEntry::CreateMessage (IMessage::MessageType,
			const QString&, const QString& body)
	{
 		IrcMessage *message = new IrcMessage (IMessage::MTChatMessage,
				IMessage::DOut,
				ISH_->GetServerID (),
				Nick_,
				Account_->GetClientConnection ().get ());

		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());

		return message;
	}

	void ServerParticipantEntry::SetMessageHistory (QObjectList messages)
	{
		AllMessages_ << messages;
	}

};
};
};
