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
#include <interfaces/azoth/irichtextmessage.h>

namespace LC
{
namespace Azoth
{
	class CoreMessage : public QObject
					  , public IMessage
					  , public IRichTextMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage LC::Azoth::IRichTextMessage)

		const Type Type_;
		const Direction Dir_;
		QObject * const Other_;
		QString Body_;
		QDateTime Date_;

		QString RichBody_;
	public:
		CoreMessage (const QString& body, const QDateTime& date,
				Type type, Direction dir, QObject *other, QObject *parent = nullptr);

		QObject* GetQObject ();
		void Send ();
		void Store ();

		Direction GetDirection () const;
		Type GetMessageType () const;
		SubType GetMessageSubType () const;

		QObject* OtherPart () const;
		QString GetOtherVariant () const;

		QString GetBody () const;
		void SetBody (const QString& body);

		EscapePolicy GetEscapePolicy () const;

		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime& timestamp);

		QString GetRichBody () const;
		void SetRichBody (const QString&);
	};
}
}
