/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCMESSAGE_H

#include <QObject>
#include <interfaces/azoth/imessage.h>

namespace LC
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
		Q_INTERFACES (LC::Azoth::IMessage)

		Type Type_;
		SubType SubType_;
		Direction Direction_;
		QString ID_;
		QString NickName_;
		Message Message_;
		ClientConnection *Connection_;

		QObject *OtherPart_;
	public:
		IrcMessage (IMessage::Type type,
				IMessage::Direction direction,
				const QString& chid,
				const QString& nickname,
				ClientConnection *conn);
		IrcMessage (const Message& msg,
				const QString& id,
				ClientConnection *conn);
		QObject* GetQObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		Type GetMessageType () const;
		SubType GetMessageSubType () const;
		void SetMessageSubType (IMessage::SubType);
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
