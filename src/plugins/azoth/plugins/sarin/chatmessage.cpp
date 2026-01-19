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
	: BaseMessage { dir, Type::ChatMessage, SubType::Other, body, contact }
	, Contact_ { contact }
	{
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

	QObject* ChatMessage::OtherPart () const
	{
		return Contact_;
	}

	QString ChatMessage::GetOtherVariant () const
	{
		return {};
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
