/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <Contact>
#include <Message>
#include <TextChannel>
#include <interfaces/structures.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iauthable.h>

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry LeechCraft::Azoth::IAuthable);

		AccountWrapper *AW_;
		Tp::ContactPtr C_;

		QList<MsgWrapper*> AllMessages_;
	public:
		EntryWrapper (Tp::ContactPtr, AccountWrapper*);

		void HandleMessage (MsgWrapper*);
		Tp::ContactPtr GetContact () const;

		QObject* GetObject ();
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType, const QString&, const QString&);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();

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
		void rawinfoChanged (const QString&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);

		void gotEntity (LeechCraft::Entity);
	};
}
}
}
