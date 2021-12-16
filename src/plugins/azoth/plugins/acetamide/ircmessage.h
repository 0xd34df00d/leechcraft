/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/imessage.h>

namespace LC::Azoth::Acetamide
{
	struct Message
	{
		QString Body_;
		QDateTime Stamp_;
		QString Nickname_;
	};

	class ClientConnection;

	class IrcMessage final : public QObject
						   , public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage)

		const Type Type_;
		const SubType SubType_;
		const Direction Direction_;
		const QString ID_;
		QString NickName_;
		Message Message_;
		ClientConnection * const Connection_;

		QObject *OtherPart_ = nullptr;
	public:
		IrcMessage (IMessage::Type type,
				IMessage::Direction direction,
				QString chid,
				QString nickname,
				ClientConnection *conn);
		IrcMessage (Message msg,
				QString id,
				ClientConnection *conn);

		QObject* GetQObject () override;
		void Send () override;
		void Store () override;
		Direction GetDirection () const override;
		Type GetMessageType () const override;
		SubType GetMessageSubType () const override;
		QObject* OtherPart () const override;
		void SetOtherPart (QObject *entry);
		QString GetID () const;
		QString GetOtherVariant () const override;
		void SetOtherVariant (const QString&);
		QString GetBody () const override;
		void SetBody (const QString&) override;
		QDateTime GetDateTime () const override;
		void SetDateTime (const QDateTime&) override;
	};
}
