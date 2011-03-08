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

#include "channelpublicmessage.h"
#include <QtDebug>
#include "channelclentry.h"
#include "channelparticipantentry.h"
#include "ircaccount.h"
#include "clientconnection.h"
#include "channelhandler.h"
#include "ircparser.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelPublicMessage::ChannelPublicMessage (const QString& msg, ChannelCLEntry *entry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (DOut)
	, Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	{
	}

	ChannelPublicMessage::ChannelPublicMessage (const QString& msg, 
			IMessage::Direction direction,
			ChannelCLEntry *entry,
			IMessage::MessageType type,
			IMessage::MessageSubType subType,
			ChannelParticipantEntry_ptr part)
	: QObject (entry)
	, ParentEntry_ (entry)
	, ParticipantEntry_ (part)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (direction)
	, Type_ (type)
	, SubType_ (subType)
	{
	}

	QObject* ChannelPublicMessage::GetObject ()
	{
		return this;
	}

	void ChannelPublicMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		/*Core::Instance ().GetIrcClient ()->PrivMessageCommand (Message_ 
				, ParentEntry_->GetChannelHandler ()->GetServerOptions ()
				, ParentEntry_->GetChannelHandler ()->GetChannelOptions ());*/
	}

	IMessage::Direction ChannelPublicMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType ChannelPublicMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::MessageSubType ChannelPublicMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	QObject* ChannelPublicMessage::OtherPart () const
	{
		switch (Direction_)
		{
		case DIn:
			return ParticipantEntry_.get ();
		case DOut:
			return ParentEntry_;
		}
	}

	QObject* ChannelPublicMessage::ParentCLEntry () const
	{
		return ParentEntry_;
	}

	QString ChannelPublicMessage::GetOtherVariant () const
	{
		return FromVariant_;
	}

	QString ChannelPublicMessage::GetBody () const
	{
		return Message_;
	}

	void ChannelPublicMessage::SetBody (const QString& msg)
	{
		Message_ = msg;
	}

	QDateTime ChannelPublicMessage::GetDateTime () const
	{
		return Datetime_;
	}

	void ChannelPublicMessage::SetDateTime (const QDateTime& dt)
	{
		Datetime_ = dt;
	}
};
};
};
