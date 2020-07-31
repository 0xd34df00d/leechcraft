/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mrimmessage.h"
#include "mrimbuddy.h"
#include "mrimaccount.h"
#include "proto/connection.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	MRIMMessage::MRIMMessage (Direction dir, Type mt, MRIMBuddy *buddy)
	: QObject (buddy)
	, Buddy_ (buddy)
	, A_ (Buddy_->GetParentAccount ())
	, Dir_ (dir)
	, MT_ (mt)
	, DateTime_ (QDateTime::currentDateTime ())
	, SendID_ (0)
	, IsDelivered_ (dir == Direction::In)
	{
		connect (A_->GetConnection (),
				SIGNAL (messageDelivered (quint32)),
				this,
				SLOT (checkMessageDelivery (quint32)));
	}

	void MRIMMessage::SetDelivered ()
	{
		if (IsDelivered_)
			return;

		IsDelivered_ = true;
		emit messageDelivered ();
	}

	QObject* MRIMMessage::GetQObject ()
	{
		return this;
	}

	void MRIMMessage::Send ()
	{
		if (Dir_ != Direction::Out)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to send incoming message";
			return;
		}

		SendID_ = A_->GetConnection ()->SendMessage (Buddy_->GetHumanReadableID (), Body_);
		Buddy_->HandleMessage (this);
	}

	void MRIMMessage::Store ()
	{
		Buddy_->HandleMessage (this);
	}

	IMessage::Direction MRIMMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type MRIMMessage::GetMessageType () const
	{
		return MT_;
	}

	IMessage::SubType MRIMMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* MRIMMessage::OtherPart () const
	{
		return Buddy_;
	}

	QString MRIMMessage::GetOtherVariant () const
	{
		return QString ();
	}

	QString MRIMMessage::GetBody () const
	{
		return Body_;
	}

	void MRIMMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime MRIMMessage::GetDateTime () const
	{
		return DateTime_;
	}

	void MRIMMessage::SetDateTime (const QDateTime& timestamp)
	{
		DateTime_ = timestamp;
	}

	bool MRIMMessage::IsDelivered () const
	{
		return IsDelivered_;
	}

	void MRIMMessage::checkMessageDelivery (quint32 id)
	{
		if (id != SendID_)
			return;

		SetDelivered ();
		disconnect (A_->GetConnection (),
				SIGNAL (messageDelivered (quint32)),
				this,
				SLOT (checkMessageDelivery (quint32)));
	}
}
}
}
