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

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	class Buddy;

	class ConvIMMessage : public QObject
						, public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage)

		Buddy * const Buddy_;
		QString Body_;
		Direction Dir_;
		QDateTime Timestamp_;
	public:
		ConvIMMessage (const QString&, Direction, Buddy*);

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
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime& timestamp);
	};
}
}
}
