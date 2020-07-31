/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircmessage.h"
#include <QtDebug>
#include <QTextDocument>
#include "clientconnection.h"
#include "core.h"
#include "ircserverhandler.h"
#include "channelsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	IrcMessage::IrcMessage (IMessage::Type type,
			IMessage::Direction dir,
			const QString& id,
			const QString& nickname,
			ClientConnection *conn)
	: Type_ (type)
	, SubType_ (SubType::Other)
	, Direction_ (dir)
	, ID_ (id)
	, NickName_ (nickname)
	, Connection_ (conn)
	, OtherPart_ (0)
	{
		Message_.Stamp_ = QDateTime::currentDateTime ();
		Message_.Nickname_ = NickName_;
	}

	IrcMessage::IrcMessage (const Message& msg,
			const QString& id, ClientConnection* conn)
	: Type_ (Type::MUCMessage)
	, SubType_ (SubType::Other)
	, Direction_ (Direction::In)
	, ID_ (id)
	, Message_ (msg)
	, Connection_ (conn)
	, OtherPart_ (0)
	{
		if (!Message_.Stamp_.isValid ())
			Message_.Stamp_ = QDateTime::currentDateTime ();
	}

	QObject* IrcMessage::GetQObject ()
	{
		return this;
	}

	void IrcMessage::Send ()
	{
		if (Direction_ == Direction::In)
		{
			qWarning () << Q_FUNC_INFO
					<< "tried to send incoming message";
			return;
		}

		switch (Type_)
		{
		case Type::ChatMessage:
		case Type::MUCMessage:
			Connection_->GetIrcServerHandler (ID_)->SendPrivateMessage (this);
			Connection_->GetIrcServerHandler (ID_)->GetChannelManager ()->SetPrivateChat (GetOtherVariant ());
			return;
		case Type::StatusMessage:
		case Type::EventMessage:
		case Type::ServiceMessage:
			qWarning () << Q_FUNC_INFO
					<< this
					<< "cannot send a service message";
			break;
		}
	}

	void IrcMessage::Store ()
	{
		ServerParticipantEntry_ptr entry =
				Connection_->GetIrcServerHandler (ID_)->
						GetParticipantEntry (GetOtherVariant ());
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

	void IrcMessage::SetMessageSubType (IMessage::SubType subtype)
	{
		SubType_ = subtype;
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
};
};
};
