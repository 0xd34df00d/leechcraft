/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXMESSAGE_H
#include <QObject>
#include <QXmppMessage.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iadvancedmessage.h>
#include <interfaces/azoth/irichtextmessage.h>

namespace LC
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
		Q_INTERFACES (LC::Azoth::IMessage
				LC::Azoth::IAdvancedMessage
				LC::Azoth::IRichTextMessage)

		Type Type_;
		SubType SubType_ = SubType::Other;
		Direction Direction_;
		QString BareJID_;
		QString Variant_;
		QDateTime DateTime_;
		QXmppMessage Message_;
		ClientConnection *Connection_;

		bool IsDelivered_ = false;
	public:
		GlooxMessage (IMessage::Type type,
				IMessage::Direction direction,
				const QString& jid,
				const QString& variant,
				ClientConnection *conn);
		GlooxMessage (const QXmppMessage& msg,
				ClientConnection *conn);

		// IMessage
		QObject* GetQObject () override;
		void Send () override;
		void Store () override;
		Direction GetDirection () const override;
		Type GetMessageType () const override;
		SubType GetMessageSubType () const override;
		void SetMessageSubType (SubType);
		QObject* OtherPart () const override;
		QString GetOtherVariant () const override;
		QString GetBody () const override;
		void SetBody (const QString&) override;
		QDateTime GetDateTime () const override;
		void SetDateTime (const QDateTime&) override;

		// IAdvancedMessage
		bool IsDelivered () const override;

		// IRichTextMessage
		QString GetRichBody () const override;
		void SetRichBody (const QString&) override;

		void SetReceiptRequested (bool);
		void SetDelivered (bool);

		void SetVariant (const QString&);

		const QXmppMessage& GetNativeMessage () const;
	signals:
		void messageDelivered () override;
	};
}
}
}

#endif
