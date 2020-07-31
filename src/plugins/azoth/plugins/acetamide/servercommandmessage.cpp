/*
	LeechCraft - modular cross-platform feature rich internet client.
	Copyright (C) 2010-2011  Oleg Linkin

 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#include "servercommandmessage.h"
#include <QTextDocument>
#include "ircserverclentry.h"
#include "ircserverhandler.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ServerCommandMessage::ServerCommandMessage (const QString& msg,
			IrcServerCLEntry *entry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (Direction::Out)
	, Type_ (Type::MUCMessage)
	, SubType_ (SubType::Other)
	{
	}

	ServerCommandMessage::ServerCommandMessage (const QString& msg,
			IMessage::Direction dir, IrcServerCLEntry *entry,
			IMessage::Type mtype, IMessage::SubType mstype)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (dir)
	, Type_ (mtype)
	, SubType_ (mstype)
	{
	}

	QObject* ServerCommandMessage::GetQObject ()
	{
		return this;
	}

	void ServerCommandMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->GetIrcServerHandler ()->
				SendMessage2Server (Message_.split (' '));
	}

	void ServerCommandMessage::Store ()
	{
		qWarning () << Q_FUNC_INFO
				<< "cannot store ServerCommandMessage";
	}

	IMessage::Direction ServerCommandMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::Type ServerCommandMessage::GetMessageType () const
	{
		return Type_;
	}

	void ServerCommandMessage::SetMessageType (IMessage::Type t)
	{
		Type_ = t;
	}

	IMessage::SubType ServerCommandMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	void ServerCommandMessage::SetMessageSubType
			(IMessage::SubType type)
	{
		SubType_ = type;
	}

	QObject* ServerCommandMessage::OtherPart () const
	{
		return ParentEntry_;
	}

	QObject* ServerCommandMessage::ParentCLEntry () const
	{
		return ParentEntry_;
	}

	QString ServerCommandMessage::GetOtherVariant () const
	{
		return FromVariant_;
	}

	QString ServerCommandMessage::GetBody () const
	{
		return Message_;
	}

	void ServerCommandMessage::SetBody (const QString& msg)
	{
		Message_ = msg;
	}

	QDateTime ServerCommandMessage::GetDateTime () const
	{
		return Datetime_;
	}

	void ServerCommandMessage::SetDateTime (const QDateTime& dt)
	{
		Datetime_ = dt;
	}
}
}
}
