/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <blist.h>
#include <conversation.h>
#include <interfaces/azoth/iclentry.h>

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	class Account;
	class ConvIMMessage;

	class Buddy : public QObject
				, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry)

		Account *Account_;
		PurpleBuddy *Buddy_;

		QString Name_;
		EntryStatus Status_;

		QString Group_;

		QList<ConvIMMessage*> Messages_;
	public:
		Buddy (PurpleBuddy*, Account*);

		QObject* GetQObject ();
		IAccount* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList& groups);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type type, const QString& variant, const QString& body);
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime& before);
		void SetChatPartState (ChatPartState state, const QString& variant);
		EntryStatus GetStatus (const QString& variant = QString ()) const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString& variant) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();

		void Send (ConvIMMessage*);
		void Store (ConvIMMessage*);
		void SetConv (PurpleConversation*);
		void HandleMessage (const char*, const char*, PurpleMessageFlags, time_t);

		PurpleBuddy* GetPurpleBuddy () const;
		void Update ();
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();
	};
}
}
}
