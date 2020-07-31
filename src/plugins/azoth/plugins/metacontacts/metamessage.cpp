/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "metamessage.h"
#include "metaentry.h"

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	MetaMessage::MetaMessage (QObject *msg, MetaEntry *entry)
	: QObject (entry)
	, Entry_ (entry)
	, MessageObj_ (msg)
	, Message_ (qobject_cast<IMessage*> (msg))
	{
	}

	QObject* MetaMessage::GetQObject ()
	{
		return this;
	}

	void MetaMessage::Send ()
	{
		Message_->Send ();
	}

	void MetaMessage::Store ()
	{
		Message_->Store ();
	}

	IMessage::Direction MetaMessage::GetDirection () const
	{
		return Message_->GetDirection ();
	}

	IMessage::Type MetaMessage::GetMessageType () const
	{
		return Message_->GetMessageType ();
	}

	IMessage::SubType MetaMessage::GetMessageSubType () const
	{
		return Message_->GetMessageSubType ();
	}

	QObject* MetaMessage::OtherPart () const
	{
		return Entry_;
	}

	QString MetaMessage::GetOtherVariant () const
	{
		return Entry_->GetMetaVariant (Message_->OtherPart (),
				Message_->GetOtherVariant ());
	}

	QString MetaMessage::GetBody () const
	{
		return Message_->GetBody ();
	}

	void MetaMessage::SetBody (const QString& body)
	{
		Message_->SetBody (body);
	}

	QDateTime MetaMessage::GetDateTime () const
	{
		return Message_->GetDateTime ();
	}

	void MetaMessage::SetDateTime (const QDateTime& dt)
	{
		Message_->SetDateTime (dt);
	}

	IMessage* MetaMessage::GetOriginalMessage () const
	{
		return Message_;
	}
}
}
}
