/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coremessage.h"
#include "dummymsgmanager.h"

namespace LC
{
namespace Azoth
{
	CoreMessage::CoreMessage (const QString& body, const QDateTime& date,
			Type type, Direction dir, QObject* other, QObject *parent)
	: QObject (parent)
	, Type_ (type)
	, Dir_ (dir)
	, Other_ (other)
	, Body_ (body)
	, Date_ (date)
	{
	}

	QObject* CoreMessage::GetQObject ()
	{
		return this;
	}

	void CoreMessage::Send ()
	{
	}

	void CoreMessage::Store ()
	{
		QMetaObject::invokeMethod (OtherPart (),
				"gotMessage",
				Q_ARG (QObject*, this));
		DummyMsgManager::Instance ().AddMessage (this);
	}

	IMessage::Direction CoreMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type CoreMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType CoreMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* CoreMessage::OtherPart () const
	{
		return Other_;
	}

	QString CoreMessage::GetOtherVariant () const
	{
		return {};
	}

	QString CoreMessage::GetBody () const
	{
		return Body_;
	}

	void CoreMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	IMessage::EscapePolicy CoreMessage::GetEscapePolicy () const
	{
		return EscapePolicy::NoEscape;
	}

	QDateTime CoreMessage::GetDateTime () const
	{
		return Date_;
	}

	void CoreMessage::SetDateTime (const QDateTime& timestamp)
	{
		Date_ = timestamp;
	}

	QString CoreMessage::GetRichBody () const
	{
		return RichBody_;
	}

	void CoreMessage::SetRichBody (const QString& richBody)
	{
		RichBody_ = richBody;
	}
}
}
