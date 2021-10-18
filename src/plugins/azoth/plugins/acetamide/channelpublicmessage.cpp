/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelpublicmessage.h"
#include <QtDebug>
#include <QTextDocument>
#include "channelclentry.h"
#include "channelhandler.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ChannelPublicMessage::ChannelPublicMessage (QString msg,
			ChannelCLEntry *entry)
	: ParentEntry_ (entry)
	, Message_ (std::move (msg))
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (Direction::Out)
	, Type_ (Type::MUCMessage)
	, SubType_ (SubType::Other)
	{
	}

	ChannelPublicMessage::ChannelPublicMessage (QString msg,
			IMessage::Direction direction,
			ChannelCLEntry *entry,
			IMessage::Type type,
			IMessage::SubType subType,
			ChannelParticipantEntry_ptr part)
	: ParentEntry_ (entry)
	, ParticipantEntry_ (std::move (part))
	, Message_ (std::move (msg))
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (direction)
	, Type_ (type)
	, SubType_ (subType)
	{
	}

	QObject* ChannelPublicMessage::GetQObject ()
	{
		return this;
	}

	void ChannelPublicMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->GetChannelHandler ()->SendPublicMessage (Message_);
	}

	void ChannelPublicMessage::Store ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->HandleMessage (this);
	}

	IMessage::Direction ChannelPublicMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::Type ChannelPublicMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType ChannelPublicMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	QObject* ChannelPublicMessage::OtherPart () const
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

	QObject* ChannelPublicMessage::ParentCLEntry () const
	{
		return ParentEntry_;
	}

	QString ChannelPublicMessage::GetOtherVariant () const
	{
		return qobject_cast<ICLEntry*> (OtherPart ())->GetEntryName ();
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
