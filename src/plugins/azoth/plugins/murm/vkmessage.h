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
#include <interfaces/azoth/irichtextmessage.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class EntryBase;

	class VkMessage : public QObject
					, public IMessage
					, public IAdvancedMessage
					, public IRichTextMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage
				LC::Azoth::IAdvancedMessage
				LC::Azoth::IRichTextMessage)

		EntryBase * const OtherPart_;
		EntryBase * const ParentCLEntry_;
		const Type Type_;
		const Direction Dir_;

		QString Body_;
		QString RichBody_;
		QDateTime TS_ = QDateTime::currentDateTime ();

		qulonglong ID_ = -1;

		bool IsRead_ = Dir_ == Direction::Out ||
				(Type_ != Type::ChatMessage && Type_ != Type::MUCMessage);

		const bool IsOurs_;
	public:
		VkMessage (bool isOurs, Direction, Type, EntryBase*, EntryBase* = nullptr);

		QObject* GetQObject ();
		void Send ();
		void Store ();

		qulonglong GetID () const;
		void SetID (qulonglong);

		bool IsRead () const;
		void SetRead ();

		QString GetRawBody () const;

		Direction GetDirection () const;
		Type GetMessageType () const;
		SubType GetMessageSubType () const;

		QObject* OtherPart () const;
		QObject* ParentCLEntry() const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString& body);
		EscapePolicy GetEscapePolicy () const;
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime& timestamp);

		bool IsDelivered () const;

		QString GetRichBody() const;
		void SetRichBody (const QString&);
	signals:
		void messageDelivered ();
	};
}
}
}
