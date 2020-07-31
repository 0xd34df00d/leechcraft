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
namespace ChatHistory
{
	class HistoryMessage : public QObject
						 , public IMessage
						 , public IRichTextMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage LC::Azoth::IRichTextMessage)

		Direction Direction_;
		QObject * const OtherPart_;
		Type Type_;
		QString Variant_;
		QString Body_;
		QDateTime DateTime_;

		QString RichBody_;

		const EscapePolicy EscPolicy_;
	public:
		HistoryMessage (Direction dir,
				QObject *other,
				Type type,
				const QString& variant,
				const QString& body,
				const QDateTime& datetime,
				const QString& richBody,
				EscapePolicy policy);

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
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);

		QString GetRichBody () const;
		void SetRichBody (const QString&);

		EscapePolicy GetEscapePolicy () const;
	};
}
}
}
