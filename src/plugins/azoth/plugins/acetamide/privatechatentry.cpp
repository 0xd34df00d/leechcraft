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

#include "privatechatentry.h"
#include "ircaccount.h"
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircmessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	PrivateChatEntry::PrivateChatEntry (const QString& nick, IrcServer *server, IrcAccount *acc)
	: Account_ (acc)
	, Server_ (server)
	, Nick_ (nick)
	{
	}

	QObject* PrivateChatEntry::GetParentAccount () const
	{
		return Account_;
	}

	QObject* PrivateChatEntry::GetParentCLEntry () const
	{
		return NULL;
	}

	ICLEntry::Features PrivateChatEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}
	
	ICLEntry::EntryType PrivateChatEntry::GetEntryType () const
	{
		return ETPrivateChat;
	}

	QString PrivateChatEntry::GetEntryName () const
	{
		return Nick_;
	}

	void PrivateChatEntry::SetEntryName (const QString& nick)
	{
		Nick_ = nick;
		emit nameChanged (Nick_);
	}
	
	QString PrivateChatEntry::GetEntryID () const
	{
		return Server_->GetHost () + 
				":" + QString::number (Server_->GetPort ()) + 
				"/" + Nick_;
	}

	QStringList PrivateChatEntry::Groups () const
	{
		return QStringList (tr ("%1 participants")
				.arg (Server_->GetServerKey ()));
	}

	void PrivateChatEntry::SetGroups (const QStringList&)
	{
	}

	QStringList PrivateChatEntry::Variants () const
	{
		return QStringList ("");
	}

	QObject* PrivateChatEntry::CreateMessage (IMessage::MessageType type,
			const QString& , const QString& body)
	{
		IrcMessage *message = new IrcMessage (IMessage::MTMUCMessage,
				IMessage::DOut,
				Account_->GetAccountName () + "/" + Nick_,
				Nick_,
				Account_->GetClientConnection ().get ());
		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());
		AllMessages_ << message;

		return message;
	}

	void PrivateChatEntry::HandleMessage (IrcMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	QString PrivateChatEntry::GetNick () const
	{
		return Nick_;
	}

	QObject* PrivateChatEntry::GetObject ()
	{
		return this;
	}

	QList<QObject*> PrivateChatEntry::GetAllMessages () const
	{
		return QList<QObject*> ();
	}

	EntryStatus PrivateChatEntry::GetStatus (const QString& variant) const
	{
		return EntryStatus (SOnline, variant);
	}

	QImage PrivateChatEntry::GetAvatar () const
	{
		return QImage ();
	}

	QString PrivateChatEntry::GetRawInfo () const
	{
		return QString ();
	}

	void PrivateChatEntry::ShowInfo ()
	{
	}

	QList<QAction*> PrivateChatEntry::GetActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> PrivateChatEntry::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}
};
};
};
