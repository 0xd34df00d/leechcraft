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
namespace Vader
{
	class MRIMBuddy;
	class MRIMAccount;

	class MRIMMessage : public QObject
					  , public IMessage
					  , public IAdvancedMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage
				LC::Azoth::IAdvancedMessage)

		MRIMBuddy *Buddy_;
		MRIMAccount *A_;
		Direction Dir_;
		Type MT_;

		QString Body_;
		QDateTime DateTime_;

		quint32 SendID_;

		bool IsDelivered_;
	public:
		MRIMMessage (Direction, Type, MRIMBuddy*);

		void SetDelivered ();

		// ICLEntry
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

		// IAdvancedMessage
		bool IsDelivered () const;
	private slots:
		void checkMessageDelivery (quint32);
	signals:
		void messageDelivered ();
	};
}
}
}
