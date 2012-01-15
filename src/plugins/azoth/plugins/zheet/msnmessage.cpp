/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "msnmessage.h"
#include <msn/message.h>
#include <msn/notificationserver.h>
#include "msnbuddyentry.h"
#include "msnaccount.h"
#include "zheetutil.h"
#include "sbmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	MSNMessage::MSNMessage (Direction dir, MessageType type, MSNBuddyEntry *entry)
	: QObject (entry)
	, Entry_ (entry)
	, Dir_ (dir)
	, MT_ (type)
	, MST_ (MSTOther)
	, DateTime_ (QDateTime::currentDateTime ())
	, IsDelivered_ (dir == DIn)
	, MsgID_ (-1)
	{
	}

	MSNMessage::MSNMessage (MSN::Message *msg, MSNBuddyEntry *entry)
	: QObject (entry)
	, Entry_ (entry)
	, Dir_ (DIn)
	, MT_ (MTChatMessage)
	, Body_ (ZheetUtil::FromStd (msg->getBody ()))
	, DateTime_ (QDateTime::currentDateTime ())
	, IsDelivered_ (true)
	{
	}

	int MSNMessage::GetID () const
	{
		return MsgID_;
	}

	void MSNMessage::SetID (int id)
	{
		MsgID_ = id;
	}

	void MSNMessage::SetDelivered ()
	{
		if (IsDelivered_)
			return;

		IsDelivered_ = true;
		emit messageDelivered ();
	}

	QObject* MSNMessage::GetObject ()
	{
		return this;
	}

	void MSNMessage::Send ()
	{
		Entry_->HandleMessage (this);

		auto acc = qobject_cast<MSNAccount*> (Entry_->GetParentAccount ());
		acc->GetSBManager ()->SendMessage (this, Entry_);
	}

	void MSNMessage::Store ()
	{
	}

	IMessage::Direction MSNMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::MessageType MSNMessage::GetMessageType () const
	{
		return MT_;
	}

	IMessage::MessageSubType MSNMessage::GetMessageSubType () const
	{
		return MST_;
	}

	QObject* MSNMessage::OtherPart () const
	{
		return Entry_;
	}

	QString MSNMessage::GetOtherVariant () const
	{
		return QString ();
	}

	QString MSNMessage::GetBody () const
	{
		return Body_;
	}

	void MSNMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime MSNMessage::GetDateTime () const
	{
		return DateTime_;
	}

	void MSNMessage::SetDateTime (const QDateTime& timestamp)
	{
		DateTime_ = timestamp;
	}

	bool MSNMessage::IsDelivered () const
	{
		return IsDelivered_;
	}
}
}
}
