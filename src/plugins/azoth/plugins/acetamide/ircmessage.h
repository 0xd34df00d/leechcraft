/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCMESSAGE_H

#include <QObject>
#include <interfaces/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	struct Message
	{
		QString Body_;
		QDateTime Stamp_;
		QString Nickname_;
	};

	class ClientConnection;

	class IrcMessage : public QObject
						, public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage);

		MessageType Type_;
		MessageSubType SubType_;
		Direction Direction_;
		QString ID_;
		QString NickName_;
		Message Message_;
		ClientConnection *Connection_;

		QObject *OtherPart_;
	public:
		IrcMessage (IMessage::MessageType type,
				IMessage::Direction direction,
				const QString& chid,
				const QString& nickname,
				ClientConnection *conn);
		IrcMessage (const Message& msg,
				const QString& id,
				ClientConnection *conn);
		QObject* GetObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;
		void SetMessageSubType (IMessage::MessageSubType);
		QObject* OtherPart () const;
		void SetOtherPart (QObject *entry);
		QString GetID () const;
		QString GetOtherVariant () const;
		void SetOtherVariant (const QString&);
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCMESSAGE_H
