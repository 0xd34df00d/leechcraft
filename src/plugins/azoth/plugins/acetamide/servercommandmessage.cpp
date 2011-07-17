/*
    LeechCraft - modular cross-platform feature rich internet client.
    Copyright (C) 2010-2011  Oleg Linkin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "servercommandmessage.h"
#include "ircserverclentry.h"
#include "ircserverhandler.h"

namespace LeechCraft
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
	, Direction_ (DOut)
	, Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	{
	}

	ServerCommandMessage::ServerCommandMessage (const QString& msg,
			IMessage::Direction dir, IrcServerCLEntry *entry,
			IMessage::MessageType mtype, IMessage::MessageSubType mstype)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (dir)
	, Type_ (mtype)
	, SubType_ (mstype)
	{
	}

	QObject* ServerCommandMessage::GetObject ()
	{
		return this;
	}

	void ServerCommandMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->GetIrcServerHandler ()->SendCommandMessage2Server (Message_);
	}

	IMessage::Direction ServerCommandMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType ServerCommandMessage::GetMessageType () const
	{
		return Type_;
	}

	void ServerCommandMessage::SetMessageType (IMessage::MessageType t)
	{
		Type_ = t;
	}

	IMessage::MessageSubType ServerCommandMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	void ServerCommandMessage::SetMessageSubType
			(IMessage::MessageSubType type)
	{
		SubType_ = type;
	}

	QObject* ServerCommandMessage::OtherPart () const
	{
		switch (Direction_)
		{
		case DIn:
		case DOut:
			return ParentEntry_;
		}
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
};
};
};