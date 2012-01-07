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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_MSNMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_MSNMESSAGE_H
#include <QObject>
#include <interfaces/imessage.h>
#include <interfaces/iadvancedmessage.h>

namespace MSN
{
	class Message;
};

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNBuddyEntry;

	class MSNMessage : public QObject
					 , public IMessage
					 , public IAdvancedMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage LeechCraft::Azoth::IAdvancedMessage);

		MSNBuddyEntry *Entry_;

		Direction Dir_;
		MessageType MT_;
		MessageSubType MST_;
		QString Body_;
		QDateTime DateTime_;

		bool IsDelivered_;

		int MsgID_;
	public:
		MSNMessage (Direction, MessageType, MSNBuddyEntry*);
		MSNMessage (MSN::Message*, MSNBuddyEntry*);

		int GetID () const;
		void SetID (int);
		void SetDelivered ();

		QObject* GetObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;
		QObject* OtherPart () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString& body);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime& timestamp);

		bool IsDelivered () const;
	signals:
		void messageDelivered ();
	};
}
}
}

#endif
