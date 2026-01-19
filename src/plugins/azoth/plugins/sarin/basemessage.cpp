/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "basemessage.h"

namespace LC::Azoth::Sarin
{
	BaseMessage::BaseMessage (Direction dir, Type type, SubType subtype, const QString& body, QObject *parent)
	: QObject { parent }
	, Body_ { body }
	, Dir_ { dir }
	, Type_ { type }
	, SubType_ { subtype }
	{
	}

	QObject* BaseMessage::GetQObject ()
	{
		return this;
	}

	IMessage::Direction BaseMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type BaseMessage::GetMessageType () const
	{
		return Type::ChatMessage;
	}

	IMessage::SubType BaseMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QString BaseMessage::GetBody () const
	{
		return Body_;
	}

	void BaseMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime BaseMessage::GetDateTime () const
	{
		return TS_;
	}

	void BaseMessage::SetDateTime (const QDateTime& timestamp)
	{
		TS_ = timestamp;
	}
}
