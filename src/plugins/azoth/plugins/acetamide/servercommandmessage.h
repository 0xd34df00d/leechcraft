/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERCOMMANDMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERCOMMANDMESSAGE_H

#include <QObject>
#include <interfaces/azoth/imessage.h>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerCLEntry;

	class ServerCommandMessage : public QObject
								, public IMessage
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMessage)

		QPointer<IrcServerCLEntry> ParentEntry_;
		QString Message_;
		QDateTime Datetime_ = QDateTime::currentDateTime ();
		QString FromVariant_;
	public:
		ServerCommandMessage (QString, IrcServerCLEntry*);

		QObject* GetQObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		Type GetMessageType () const;
		SubType GetMessageSubType () const;
		/** Since it's outgoing message, the other part
		 * always equals to the room entry.
		 */
		QObject* OtherPart () const;
		QObject* ParentCLEntry () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString&);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERCOMMANDMESSAGE_H
