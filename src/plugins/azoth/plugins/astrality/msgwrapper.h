/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <ContactMessenger>
#include <Types>
#include <interfaces/structures.h>
#include <interfaces/azoth/imessage.h>

namespace LC
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
		Q_INTERFACES (LC::Azoth::IMessage);

		Tp::ContactMessengerPtr Messenger_;
		EntryWrapper *Entry_;

		QString Body_;
		QDateTime DT_;
		Direction Dir_;
		Type MT_;
		SubType MST_;
	public:
		MsgWrapper (const Tp::ReceivedMessage&,
				Tp::ContactMessengerPtr, EntryWrapper*);
		MsgWrapper (const QString&, Direction,
				Tp::ContactMessengerPtr, EntryWrapper*,
				Type, SubType = SubType::Other);

		QObject* GetQObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		Type GetMessageType () const;
		SubType GetMessageSubType () const;
		QObject* OtherPart () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime() const;
		void SetDateTime (const QDateTime&);
	private slots:
		void handleMessageSent (Tp::PendingOperation*);
	signals:
		void gotEntity (const LC::Entity&);
	};
}
}
}
