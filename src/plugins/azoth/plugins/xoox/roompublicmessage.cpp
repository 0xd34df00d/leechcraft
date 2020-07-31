/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "roompublicmessage.h"
#include <QTextDocument>
#include <QtDebug>
#include <QXmppMessage.h>
#include <QXmppClient.h>
#include "roomclentry.h"
#include "roomparticipantentry.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "roomhandler.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	RoomPublicMessage::RoomPublicMessage (const QString& msg, RoomCLEntry *entry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (Direction::Out)
	, Type_ (Type::MUCMessage)
	, SubType_ (SubType::Other)
	{
	}

	RoomPublicMessage::RoomPublicMessage (const QString& msg,
			IMessage::Direction direction,
			RoomCLEntry *entry,
			IMessage::Type type,
			IMessage::SubType subType,
			RoomParticipantEntry_ptr part)
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

	RoomPublicMessage::RoomPublicMessage (const QXmppMessage& msg,
			RoomCLEntry *entry,
			RoomParticipantEntry_ptr partEntry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, ParticipantEntry_ (partEntry)
	, Message_ (msg.body ())
	, Datetime_ (msg.stamp ().isValid () ? msg.stamp ().toLocalTime () : QDateTime::currentDateTime ())
	, Direction_ (Direction::In)
	, Type_ (Type::MUCMessage)
	, SubType_ (SubType::Other)
	, XHTML_ (msg.xhtml ())
	{
		std::tie (FromJID_, FromVariant_) = ClientConnection::Split (msg.from ());
	}

	void RoomPublicMessage::SetParticipantEntry (const RoomParticipantEntry_ptr& entry)
	{
		ParticipantEntry_ = entry;
	}

	QObject* RoomPublicMessage::GetQObject ()
	{
		return this;
	}

	void RoomPublicMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		const auto client = ParentEntry_->GetParentAccount ()->GetClientConnection ()->GetClient ();

		QXmppMessage msg;
		msg.setBody (Message_);
		msg.setTo (ParentEntry_->GetRoomHandler ()->GetRoomJID ());
		msg.setType (QXmppMessage::GroupChat);
		msg.setXhtml (XHTML_);
		client->sendPacket (msg);
	}

	void RoomPublicMessage::Store ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->HandleMessage (this);
	}

	IMessage::Direction RoomPublicMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::Type RoomPublicMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType RoomPublicMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	QObject* RoomPublicMessage::OtherPart () const
	{
		switch (Direction_)
		{
		case Direction::In:
			return ParticipantEntry_.get ();
		case Direction::Out:
			return ParentEntry_;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown direction"
				<< static_cast<int> (Direction_);
		return ParentEntry_;
	}

	QObject* RoomPublicMessage::ParentCLEntry () const
	{
		return ParentEntry_;
	}

	QString RoomPublicMessage::GetOtherVariant() const
	{
		return FromVariant_;
	}

	QString RoomPublicMessage::GetBody () const
	{
		return Message_;
	}

	void RoomPublicMessage::SetBody (const QString& msg)
	{
		Message_ = msg;
	}

	QDateTime RoomPublicMessage::GetDateTime () const
	{
		return Datetime_;
	}

	void RoomPublicMessage::SetDateTime (const QDateTime& dt)
	{
		Datetime_ = dt;
	}

	QString RoomPublicMessage::GetRichBody () const
	{
		return XHTML_;
	}

	void RoomPublicMessage::SetRichBody (const QString& xhtml)
	{
		XHTML_ = xhtml;
	}
}
}
}
