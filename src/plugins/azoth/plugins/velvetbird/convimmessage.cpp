/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "convimmessage.h"
#include "buddy.h"

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	ConvIMMessage::ConvIMMessage (const QString& body, Direction dir, Buddy *buddy)
	: QObject (buddy)
	, Buddy_ (buddy)
	, Body_ (body)
	, Dir_ (dir)
	, Timestamp_ (QDateTime::currentDateTime ())
	{
	}

	QObject* ConvIMMessage::GetQObject ()
	{
		return this;
	}

	void ConvIMMessage::Send ()
	{
		Buddy_->Send (this);
	}

	void ConvIMMessage::Store ()
	{
		Buddy_->Store (this);
	}

	IMessage::Direction ConvIMMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type ConvIMMessage::GetMessageType () const
	{
		return Type::ChatMessage;
	}

	IMessage::SubType ConvIMMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* ConvIMMessage::OtherPart () const
	{
		return Buddy_;
	}

	QString ConvIMMessage::GetOtherVariant () const
	{
		return QString ();
	}

	QString ConvIMMessage::GetBody () const
	{
		return Body_;
	}

	void ConvIMMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime ConvIMMessage::GetDateTime () const
	{
		return Timestamp_;
	}

	void ConvIMMessage::SetDateTime (const QDateTime& timestamp)
	{
		Timestamp_ = timestamp;
	}
}
}
}
