/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iadvancedmessage.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class ToxContact;

	class ChatMessage : public QObject
					  , public IMessage
					  , public IAdvancedMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage
				LC::Azoth::IAdvancedMessage)

		ToxContact * const Contact_;
		const Direction Dir_;

		QString Body_;
		QDateTime TS_ = QDateTime::currentDateTime ();

		bool IsDelivered_ = false;
	public:
		ChatMessage (const QString&, Direction, ToxContact*);

		QObject* GetQObject () override;
		void Send () override;
		void Store () override;

		Direction GetDirection () const override;
		Type GetMessageType () const override;
		SubType GetMessageSubType () const override;
		QObject* OtherPart () const override;
		QString GetOtherVariant () const override;

		QString GetBody () const override;
		void SetBody (const QString& body) override;
		QDateTime GetDateTime () const override;
		void SetDateTime (const QDateTime& timestamp) override;

		bool IsDelivered () const override;

		void SetDelivered ();
	signals:
		void messageDelivered () override;
	};
}
}
}
