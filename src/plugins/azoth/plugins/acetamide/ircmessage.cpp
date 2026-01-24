/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircmessage.h"
#include "clientconnection.h"
#include "ircserverhandler.h"
#include "channelsmanager.h"

namespace LC::Azoth::Acetamide
{
	IrcMessage::IrcMessage (const OutgoingMessage& message,
			QString id,
			QString nickname,
			ClientConnection *conn)
	: Type_ { Type::ChatMessage }
	, SubType_ { SubType::Other }
	, Direction_ { Direction::Out }
	, ID_ { std::move (id) }
	, NickName_ { std::move (nickname) }
	, Connection_ { conn }
	{
		Message_.Stamp_ = QDateTime::currentDateTime ();
		Message_.Body_ = message.Body_;
		Message_.Nickname_ = NickName_;

		const auto handler = Connection_->GetIrcServerHandler (ID_);
		handler->SendPrivateMessage (this);
		handler->GetChannelManager ()->SetPrivateChat (GetOtherVariant ());
	}

	IrcMessage::IrcMessage (Message msg, QString id, ClientConnection* conn)
	: Type_ { msg.Type_ }
	, SubType_ { SubType::Other }
	, Direction_ { Direction::In }
	, ID_ { std::move (id) }
	, Message_ { std::move (msg) }
	, Connection_ { conn }
	{
		if (!Message_.Stamp_.isValid ())
			Message_.Stamp_ = QDateTime::currentDateTime ();
	}

	QObject* IrcMessage::GetQObject ()
	{
		return this;
	}

	void IrcMessage::Store ()
	{
		const auto entry = Connection_->GetIrcServerHandler (ID_)->GetParticipantEntry (GetOtherVariant ());
		entry->HandleMessage (this);
	}

	IMessage::Direction IrcMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::Type IrcMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType IrcMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	QObject* IrcMessage::OtherPart () const
	{
		return OtherPart_;
	}

	void IrcMessage::SetOtherPart (QObject *entry)
	{
		OtherPart_ = entry;
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
}
