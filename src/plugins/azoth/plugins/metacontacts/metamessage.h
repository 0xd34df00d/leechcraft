/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_METAMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_METAMESSAGE_H
#include <QObject>
#include <interfaces/azoth/imessage.h>

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	class MetaEntry;

	class MetaMessage : public QObject
					  , public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage)

		MetaEntry *Entry_;
		QObject *MessageObj_;
		IMessage *Message_;
	public:
		MetaMessage (QObject*, MetaEntry*);

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

		IMessage* GetOriginalMessage () const;
	};
}
}
}

#endif
