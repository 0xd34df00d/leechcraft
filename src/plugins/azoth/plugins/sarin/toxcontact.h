/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/iclentry.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class ToxAccount;
	class ChatMessage;

	class ToxContact : public QObject
					 , public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry)

		const QByteArray Pubkey_;
		ToxAccount * const Acc_;

		QList<ChatMessage*> AllMessages_;

		QString PublicName_;

		EntryStatus Status_;
	public:
		ToxContact (const QByteArray& pubkey, ToxAccount *account);

		const QByteArray& GetPubKey () const;

		QObject* GetQObject () override;
		IAccount* GetParentAccount () const override;
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;

		QString GetEntryName () const override;
		void SetEntryName (const QString& name) override;
		QString GetEntryID () const override;
		QString GetHumanReadableID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList& groups) override;

		QStringList Variants () const override;

		IMessage* CreateMessage (IMessage::Type, const QString&, const QString&) override;
		QList<IMessage*> GetAllMessages () const override;
		void PurgeMessages (const QDateTime&) override;

		void SetChatPartState (ChatPartState, const QString&) override;

		EntryStatus GetStatus (const QString&) const override;
		void ShowInfo () override;
		QList<QAction*> GetActions () const override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;

		void MarkMsgsRead () override;
		void ChatTabClosed () override;

		void SetStatus (const EntryStatus&);
		void SetTyping (bool);

		void HandleMessage (ChatMessage*);
		void SendMessage (ChatMessage*);
	signals:
		void gotMessage (QObject*) override;
		void statusChanged (const EntryStatus&, const QString&) override;
		void availableVariantsChanged (const QStringList&) override;
		void nameChanged (const QString&) override;
		void groupsChanged (const QStringList&) override;
		void chatPartStateChanged (const ChatPartState&, const QString&) override;
		void permsChanged () override;
		void entryGenerallyChanged () override;
	};
}
}
}
