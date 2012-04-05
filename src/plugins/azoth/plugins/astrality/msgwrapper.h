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

#pragma once

#include <QObject>
#include <ContactMessenger>
#include <Types>
#include <interfaces/structures.h>
#include <interfaces/azoth/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	class EntryWrapper;

	class MsgWrapper : public QObject
					 , public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage);

		Tp::ContactMessengerPtr Messenger_;
		EntryWrapper *Entry_;

		QString Body_;
		QDateTime DT_;
		Direction Dir_;
		MessageType MT_;
		MessageSubType MST_;
	public:
		MsgWrapper (const Tp::ReceivedMessage&,
				Tp::ContactMessengerPtr, EntryWrapper*);
		MsgWrapper (const QString&, Direction,
				Tp::ContactMessengerPtr, EntryWrapper*,
				MessageType, MessageSubType = MSTOther);

		QObject* GetObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;
		QObject* OtherPart () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime() const;
		void SetDateTime (const QDateTime&);
	private slots:
		void handleMessageSent (Tp::PendingOperation*);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
