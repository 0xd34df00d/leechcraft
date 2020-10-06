/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chatmessage.h"
#include "toxcontact.h"

namespace LC::Azoth::Sarin
{
	ChatMessage::ChatMessage (const QString& body, IMessage::Direction dir, ToxContact *contact)
	: QObject { contact }
	, Contact_ { contact }
	, Dir_ { dir }
	, Body_ { body }
	{
	}

	QObject* ChatMessage::GetQObject ()
	{
		return this;
	}

	void ChatMessage::Send ()
	{
		Contact_->SendMessage (this);
		Store ();
	}

	void ChatMessage::Store ()
	{
		Contact_->HandleMessage (this);
	}

	IMessage::Direction ChatMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type ChatMessage::GetMessageType () const
	{
		return Type::ChatMessage;
	}

	IMessage::SubType ChatMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* ChatMessage::OtherPart () const
	{
		return Contact_;
	}

	QString ChatMessage::GetOtherVariant () const
	{
		return "";
	}

	QString ChatMessage::GetBody () const
	{
		return Body_;
	}

	void ChatMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime ChatMessage::GetDateTime () const
	{
		return TS_;
	}

	void ChatMessage::SetDateTime (const QDateTime& timestamp)
	{
		TS_ = timestamp;
	}

	bool ChatMessage::IsDelivered () const
	{
		return IsDelivered_;
	}

	void ChatMessage::SetDelivered ()
	{
		if (IsDelivered_)
			return;

		IsDelivered_ = true;
		emit messageDelivered ();
	}
}
