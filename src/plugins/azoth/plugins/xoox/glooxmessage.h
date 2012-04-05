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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXMESSAGE_H
#include <QObject>
#include <QXmppMessage.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iadvancedmessage.h>
#include <interfaces/azoth/irichtextmessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxCLEntry;
	class ClientConnection;

	class GlooxMessage : public QObject
					   , public IMessage
					   , public IAdvancedMessage
					   , public IRichTextMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage
				LeechCraft::Azoth::IAdvancedMessage
				LeechCraft::Azoth::IRichTextMessage)

		MessageType Type_;
		MessageSubType SubType_;
		Direction Direction_;
		QString BareJID_;
		QString Variant_;
		QDateTime DateTime_;
		QXmppMessage Message_;
		ClientConnection *Connection_;

		bool IsDelivered_;
	public:
		GlooxMessage (IMessage::MessageType type,
				IMessage::Direction direction,
				const QString& jid,
				const QString& variant,
				ClientConnection *conn);
		GlooxMessage (const QXmppMessage& msg,
				ClientConnection *conn);

		// IMessage
		QObject* GetObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;
		void SetMessageSubType (MessageSubType);
		QObject* OtherPart () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);

		// IAdvancedMessage
		bool IsDelivered () const;

		// IRichTextMessage
		QString GetRichBody () const;
		void SetRichBody (const QString&);

		void SetDelivered (bool);

		QXmppMessage GetMessage () const;
	signals:
		void messageDelivered ();
	};
}
}
}

#endif
