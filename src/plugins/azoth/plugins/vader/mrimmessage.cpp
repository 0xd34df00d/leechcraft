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

#include "mrimmessage.h"
#include "mrimbuddy.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	MRIMMessage::MRIMMessage (Direction dir, MessageType mt, MRIMBuddy *buddy)
	: QObject (buddy)
	, Buddy_ (buddy)
	, Dir_ (dir)
	, MT_ (mt)
	, DateTime_ (QDateTime::currentDateTime ())
	{
	}

	QObject* MRIMMessage::GetObject ()
	{
		return this;
	}

	void MRIMMessage::Send ()
	{
	}

	void MRIMMessage::Store ()
	{
	}

	IMessage::Direction MRIMMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::MessageType MRIMMessage::GetMessageType () const
	{
		return MT_;
	}

	IMessage::MessageSubType MRIMMessage::GetMessageSubType () const
	{
		return MSTOther;
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
}
}
}
