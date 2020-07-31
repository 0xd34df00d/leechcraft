/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ENTRYBASE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ENTRYBASE_H

#include <QObject>
#include <QImage>
#include <QVariant>
#include <interfaces/azoth/iclentry.h>
#include "localtypes.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccount;
	class IrcMessage;
	class VCardDialog;

	class EntryBase : public QObject
					, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry)
	protected:
		QList<IMessage*> AllMessages_;
		EntryStatus CurrentStatus_;
		QList<QAction*> Actions_;

		IrcAccount *Account_;
		VCardDialog *VCardDialog_;

	public:
		EntryBase (IrcAccount* = 0);
		virtual ~EntryBase ();

		QObject* GetQObject ();
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;

		void MarkMsgsRead ();
		void ChatTabClosed ();

		virtual QString GetEntryID () const = 0;

		void HandleMessage (IrcMessage*);
		void SetStatus (const EntryStatus&);
		void SetAvatar (const QByteArray&);
		void SetAvatar (const QImage&);
		void SetRawInfo (const QString&);
		void SetInfo (const WhoIsMessage& msg);

	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void chatTabClosed ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ENTRYBASE_H
