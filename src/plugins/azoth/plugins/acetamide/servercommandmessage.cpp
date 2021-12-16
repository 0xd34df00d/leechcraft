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

namespace LC::Azoth::Acetamide
{
	ServerCommandMessage::ServerCommandMessage (QString msg,
			IrcServerCLEntry *entry)
	: QObject { entry }
	, ParentEntry_ { entry }
	, Message_ { std::move (msg) }
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

		ParentEntry_->GetIrcServerHandler ()->SendMessage2Server (Message_);
	}

	void ServerCommandMessage::Store ()
	{
		qWarning () << Q_FUNC_INFO
				<< "cannot store ServerCommandMessage";
	}

	IMessage::Direction ServerCommandMessage::GetDirection () const
	{
		return Direction::Out;
	}

	IMessage::Type ServerCommandMessage::GetMessageType () const
	{
		return Type::MUCMessage;
	}

	IMessage::SubType ServerCommandMessage::GetMessageSubType () const
	{
		return SubType::Other;
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
