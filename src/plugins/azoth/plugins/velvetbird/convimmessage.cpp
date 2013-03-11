/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "convimmessage.h"
#include "buddy.h"

namespace LeechCraft
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

	QObject* ConvIMMessage::GetObject ()
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

	IMessage::MessageType ConvIMMessage::GetMessageType () const
	{
		return MTChatMessage;
	}

	IMessage::MessageSubType ConvIMMessage::GetMessageSubType () const
	{
		return MSTOther;
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
