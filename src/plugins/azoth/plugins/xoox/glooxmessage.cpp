/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "glooxmessage.h"
#include <QtDebug>
#include <gloox/messagesession.h>
#include <gloox/message.h>
#include "glooxclentry.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	GlooxMessage::GlooxMessage (IMessage::MessageType type,
			IMessage::Direction direction,
			QObject *entry,
			gloox::MessageSession *session)
	: Type_ (type)
	, Direction_ (direction)
	, Entry_ (entry)
	, Variant_ (QString::fromUtf8 (session->target ().resource ().c_str ()))
	, Session_ (session)
	{
	}

	GlooxMessage::GlooxMessage (const gloox::Message& message,
			QObject *entry,
			gloox::MessageSession *session)
	: Type_ (MTChatMessage)
	, Direction_ (DIn)
	, Entry_ (entry)
	, Body_ (QString::fromUtf8 (message.body ().c_str ()))
	, Variant_ (QString::fromUtf8 (session->target ().resource ().c_str ()))
	, Session_ (session)
	{
	}

	QObject* GlooxMessage::GetObject ()
	{
		return this;
	}

	void GlooxMessage::Send ()
	{
		if (Direction_ == DIn)
		{
			qWarning () << Q_FUNC_INFO
					<< "tried to send incoming message";
			return;
		}

		switch (Type_)
		{
		case MTChatMessage:
		case MTMUCMessage:
			Session_->send (Body_.toUtf8 ().constData (), std::string ());
			return;
		case MTServiceMessage:
			qWarning () << Q_FUNC_INFO
					<< this
					<< "cannot send a service message";
			break;
		}
	}

	IMessage::Direction GlooxMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType GlooxMessage::GetMessageType () const
	{
		return Type_;
	}

	QObject* GlooxMessage::OtherPart () const
	{
		return Entry_;
	}

	QString GlooxMessage::GetOtherVariant () const
	{
		return Variant_;
	}

	QString GlooxMessage::GetBody () const
	{
		return Body_;
	}

	void GlooxMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime GlooxMessage::GetDateTime () const
	{
		return DateTime_;
	}

	void GlooxMessage::SetDateTime (const QDateTime& dateTime)
	{
		DateTime_ = dateTime;
	}
}
}
}
}
}
