/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <Contact>
#include <Message>
#include <TextChannel>
#include <interfaces/structures.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iauthable.h>

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	class AccountWrapper;
	class MsgWrapper;

	class EntryWrapper : public QObject
					   , public ICLEntry
					   , public IAuthable
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry LC::Azoth::IAuthable);

		AccountWrapper *AW_;
		Tp::ContactPtr C_;

		QList<MsgWrapper*> AllMessages_;
	public:
		EntryWrapper (Tp::ContactPtr, AccountWrapper*);

		void HandleMessage (MsgWrapper*);
		Tp::ContactPtr GetContact () const;

		QObject* GetQObject ();
		IAccount* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type, const QString&, const QString&);
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QImage GetAvatar () const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();

		AuthStatus GetAuthStatus () const;
		void ResendAuth (const QString&);
		void RevokeAuth (const QString&);
		void Unsubscribe (const QString&);
		void RerequestAuth (const QString&);
	private slots:
		void handlePresenceChanged ();
		void handleAvatarDataChanged ();
		void handleContactInfo (Tp::PendingOperation*);
		void handlePublishStateChanged (Tp::Contact::PresenceState, const QString&);
		void handleSubStateChanged (Tp::Contact::PresenceState);
		void handleMessageReceived (const Tp::ReceivedMessage&, Tp::TextChannelPtr);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);

		void gotEntity (LC::Entity);
	};
}
}
}
