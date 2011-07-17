/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "metamessage.h"
#include "metaentry.h"

namespace LeechCraft
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
	
	QObject* MetaMessage::GetObject ()
	{
		return this;
	}

	void MetaMessage::Send ()
	{
		Message_->Send ();
	}

	IMessage::Direction MetaMessage::GetDirection () const
	{
		return Message_->GetDirection ();
	}

	IMessage::MessageType MetaMessage::GetMessageType () const
	{
		return Message_->GetMessageType ();
	}

	IMessage::MessageSubType MetaMessage::GetMessageSubType () const
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
}
}
}
