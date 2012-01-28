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

#include "historymessage.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	HistoryMessage::HistoryMessage (IMessage::Direction dir,
			QObject *otherPart,
			const QString& variant,
			const QString& body,
			const QDateTime& dt)
	: Direction_ (dir)
	, OtherPart_ (otherPart)
	, Variant_ (variant)
	, Body_ (body)
	, DateTime_ (dt)
	{
	}

	QObject* HistoryMessage::GetObject ()
	{
		return this;
	}

	void HistoryMessage::Send ()
	{
		qWarning () << Q_FUNC_INFO
				<< "unable to send history message";
	}

	void HistoryMessage::Store ()
	{
		qWarning () << Q_FUNC_INFO
				<< "unable to store history message";
	}

	IMessage::Direction HistoryMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType HistoryMessage::GetMessageType () const
	{
		return MTChatMessage;
	}

	IMessage::MessageSubType HistoryMessage::GetMessageSubType () const
	{
		return MSTOther;
	}

	QObject* HistoryMessage::OtherPart () const
	{
		return OtherPart_;
	}

	QString HistoryMessage::GetOtherVariant () const
	{
		return Variant_;
	}

	QString HistoryMessage::GetBody () const
	{
		return Body_;
	}

	void HistoryMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime HistoryMessage::GetDateTime () const
	{
		return DateTime_;
	}

	void HistoryMessage::SetDateTime (const QDateTime& dt)
	{
		DateTime_ = dt;
	}
}
}
}
