/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glooxmessage.h"
#include <QTextDocument>
#include <QtDebug>
#include <QXmppClient.h>
#include "glooxclentry.h"
#include "clientconnection.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	GlooxMessage::GlooxMessage (IMessage::Type type,
			IMessage::Direction dir,
			const QString& jid,
			const QString& variant,
			ClientConnection *conn)
	: Type_ (type)
	, Direction_ (dir)
	, BareJID_ (jid)
	, Variant_ (variant)
	, DateTime_ (QDateTime::currentDateTime ())
	, Connection_ (conn)
	{
		const QString& remoteJid = variant.isEmpty () ?
				jid :
				jid + "/" + variant;
		if (type == Type::ChatMessage && variant.isEmpty ())
		{
			QObject *object = Connection_->GetCLEntry (jid, variant);
			Variant_ = qobject_cast<ICLEntry*> (object)->Variants ().value (0);
		}
		Message_.setTo (dir == Direction::In ? conn->GetOurJID () : remoteJid);
	}

	GlooxMessage::GlooxMessage (const QXmppMessage& message,
			ClientConnection *conn)
	: Type_ (Type::ChatMessage)
	, Direction_ (Direction::In)
	, Message_ (message)
	, Connection_ (conn)
	{
		std::tie (BareJID_, Variant_) = ClientConnection::Split (message.from ());

		if (!Message_.stamp ().isValid ())
			Message_.setStamp (QDateTime::currentDateTime ());
		else
			Message_.setStamp (Message_.stamp ().toLocalTime ());
		DateTime_ = Message_.stamp ();
	}

	QObject* GlooxMessage::GetQObject ()
	{
		return this;
	}

	void GlooxMessage::Send ()
	{
		if (Direction_ == Direction::In)
		{
			qWarning () << Q_FUNC_INFO
					<< "tried to send incoming message";
			return;
		}

		switch (Type_)
		{
		case Type::ChatMessage:
		case Type::MUCMessage:
			Connection_->GetAccount ()->SendMessage (*this);
			qobject_cast<ICLEntry*> (OtherPart ())->gotMessage (this);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< this
					<< "cannot send a message of type"
					<< static_cast<int> (Type_);
			break;
		}
	}

	void GlooxMessage::Store ()
	{
		qobject_cast<ICLEntry*> (OtherPart ())->gotMessage (this);
	}

	IMessage::Direction GlooxMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::Type GlooxMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType GlooxMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	void GlooxMessage::SetMessageSubType (IMessage::SubType subType)
	{
		SubType_ = subType;
	}

	QObject* GlooxMessage::OtherPart () const
	{
		return Connection_->GetCLEntry (BareJID_, Variant_);
	}

	QString GlooxMessage::GetOtherVariant () const
	{
		return Variant_;
	}

	QString GlooxMessage::GetBody () const
	{
		return Message_.body ();
	}

	void GlooxMessage::SetBody (const QString& body)
	{
		Message_.setBody (body);
	}

	QDateTime GlooxMessage::GetDateTime () const
	{
		return DateTime_;
	}

	void GlooxMessage::SetDateTime (const QDateTime& dateTime)
	{
		DateTime_ = dateTime;
		if (Direction_ == Direction::In)
			Message_.setStamp (dateTime);
	}

	bool GlooxMessage::IsDelivered () const
	{
		return IsDelivered_;
	}

	QString GlooxMessage::GetRichBody () const
	{
		return Message_.xhtml ();
	}

	void GlooxMessage::SetRichBody (const QString& html)
	{
		Message_.setXhtml (html);
	}

	void GlooxMessage::SetReceiptRequested (bool request)
	{
		Message_.setReceiptRequested (request);
	}

	void GlooxMessage::SetDelivered (bool delivered)
	{
		IsDelivered_ = delivered;
		if (delivered)
			emit messageDelivered ();
	}

	void GlooxMessage::SetVariant (const QString& variant)
	{
		if (variant == Variant_)
			return;

		Variant_ = variant;

		if (Direction_ == Direction::In)
			Message_.setFrom (Variant_.isEmpty () ?
					BareJID_ :
					(BareJID_ + '/' + Variant_));
	}

	const QXmppMessage& GlooxMessage::GetNativeMessage () const
	{
		return Message_;
	}
}
}
}
