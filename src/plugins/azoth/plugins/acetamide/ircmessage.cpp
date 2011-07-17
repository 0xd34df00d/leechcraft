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

#include "ircmessage.h"
#include <QtDebug>
#include "clientconnection.h"
#include "core.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcMessage::IrcMessage (IMessage::MessageType type,
			IMessage::Direction dir,
			const QString& id,
			const QString& nickname,
			ClientConnection *conn)
	: Type_ (type)
	, SubType_ (MSTOther)
	, Direction_ (dir)
	, ID_ (id)
	, NickName_ (nickname)
	, Connection_ (conn)
	{
		Message_.Stamp_ = QDateTime::currentDateTime ();
		Message_.Nickname_ = NickName_;
	}

	IrcMessage::IrcMessage (const Message& msg,
			const QString& id, ClientConnection* conn)
	: Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	, Direction_ (DIn)
	, ID_ (id)
	, Message_ (msg)
	, Connection_ (conn)
	{
		if (!Message_.Stamp_.isValid ())
			Message_.Stamp_ = QDateTime::currentDateTime ();
	}

	QObject* IrcMessage::GetObject ()
	{
		return this;
	}

	void IrcMessage::Send ()
	{
		if (Direction_ == DIn)
		{
			qWarning () << Q_FUNC_INFO
					<< "tried to send incoming message";
			return;
		}

		switch (Type_)
		{
		case MTChatMessage:
		case MTMUCMessage:
				Connection_->GetIrcServerHandler (ID_)->
						SendPrivateMessage (this);
			return;
		case MTStatusMessage:
		case MTEventMessage:
		case MTServiceMessage:
			qWarning () << Q_FUNC_INFO
					<< this
					<< "cannot send a service message";
			break;
		}
	}

	IMessage::Direction IrcMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType IrcMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::MessageSubType IrcMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	void IrcMessage::SetMessageSubType (IMessage::MessageSubType subtype)
	{
		SubType_ = subtype;
	}

	QObject* IrcMessage::OtherPart () const
	{
		return Connection_->GetCLEntry (ID_, Message_.Nickname_);
	}

	QString IrcMessage::GetID () const
	{
		return ID_;
	}

	QString IrcMessage::GetOtherVariant () const
	{
		return Message_.Nickname_;
	}

	void IrcMessage::SetOtherVariant (const QString& nick)
	{
		Message_.Nickname_ = nick;
	}

	QString IrcMessage::GetBody () const
	{
		return Message_.Body_;
	}

	void IrcMessage::SetBody (const QString& body)
	{
		Message_.Body_ = body;
	}

	QDateTime IrcMessage::GetDateTime () const
	{
		return Message_.Stamp_;
	}

	void IrcMessage::SetDateTime (const QDateTime& dateTime)
	{
		Message_.Stamp_ = dateTime;
	}
};
};
};
