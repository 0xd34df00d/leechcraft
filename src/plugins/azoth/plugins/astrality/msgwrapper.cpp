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

#include "msgwrapper.h"
#include <PendingSendMessage>
#include <util/util.h>
#include "entrywrapper.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	MsgWrapper::MsgWrapper (const Tp::ReceivedMessage& msg,
			Tp::ContactMessengerPtr messenger, EntryWrapper *entry)
	: QObject (entry)
	, Messenger_ (messenger)
	, Entry_ (entry)
	, Body_ (msg.text ())
	, DT_ (msg.received ())
	, Dir_ (DIn)
	, MT_ (MTChatMessage)
	, MST_ (MSTOther)
	{
	}

	MsgWrapper::MsgWrapper (const QString& body, Direction dir,
			Tp::ContactMessengerPtr messenger, EntryWrapper *entry,
			MessageType mt, MessageSubType mst)
	: QObject (entry)
	, Messenger_ (messenger)
	, Entry_ (entry)
	, Body_ (body)
	, DT_ (QDateTime::currentDateTime ())
	, Dir_ (dir)
	, MT_ (mt)
	, MST_ (mst)
	{
		connect (this,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				Entry_,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	QObject* MsgWrapper::GetObject ()
	{
		return this;
	}

	void MsgWrapper::Send ()
	{
		Store ();
		connect (Messenger_->sendMessage (Body_),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleMessageSent (Tp::PendingOperation*)));
	}

	void MsgWrapper::Store ()
	{
		Entry_->HandleMessage (this);
	}

	IMessage::Direction MsgWrapper::GetDirection () const
	{
		return Dir_;
	}

	IMessage::MessageType MsgWrapper::GetMessageType () const
	{
		return MT_;
	}

	IMessage::MessageSubType MsgWrapper::GetMessageSubType () const
	{
		return MST_;
	}

	QObject* MsgWrapper::OtherPart () const
	{
		return Entry_;
	}

	QString MsgWrapper::GetOtherVariant () const
	{
		return QString ();
	}

	QString MsgWrapper::GetBody () const
	{
		return Body_;
	}

	void MsgWrapper::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime MsgWrapper::GetDateTime () const
	{
		return DT_;
	}

	void MsgWrapper::SetDateTime (const QDateTime& dt)
	{
		DT_ = dt;
	}

	void MsgWrapper::handleMessageSent (Tp::PendingOperation *po)
	{
		auto psm = qobject_cast<Tp::PendingSendMessage*> (po);
		if (psm->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< psm->errorName ()
					<< psm->errorMessage ();
			emit gotEntity (Util::MakeNotification ("Azoth",
					tr ("Unable to send message to %1: %2 (%3).")
						.arg (Entry_->GetEntryName ())
						.arg (po->errorName ())
						.arg (po->errorMessage ()),
					PCritical_));
			return;
		}
	}
}
}
}
