/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <Types>
#include <Account>
#include <ContactMessenger>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iextselfinfoaccount.h>

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	class EntryWrapper;

	class AccountWrapper : public QObject
						 , public IAccount
						 , public IExtSelfInfoAccount
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAccount LC::Azoth::IExtSelfInfoAccount);

		Tp::AccountPtr A_;
		const ICoreProxy_ptr Proxy_;
		QList<EntryWrapper*> Entries_;

		QMap<QString, Tp::ContactMessengerPtr> Messengers_;
	public:
		struct Settings
		{
			bool Autodisconnect_;
		};
	private:
		Settings S_;
	public:
		AccountWrapper (Tp::AccountPtr, const ICoreProxy_ptr&, QObject*);

		// IAccount
		QObject* GetQObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString&);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager() const;

		// IExtSelfInfoAccount
		QObject* GetSelfContact () const;
		QIcon GetAccountIcon () const;

		Tp::ContactMessengerPtr GetMessenger (const QString&);
		QString GetOurID () const;
		void Shutdown ();
		void RemoveThis ();
		void SetSettings (const Settings&);
	private:
		void HandleAuth (bool failure);
		EntryWrapper* CreateEntry (Tp::ContactPtr);

		void LoadSettings ();
		void SaveSettings ();
	private slots:
		void handleAccountReady (Tp::PendingOperation* = 0);

		void handleEnabled (Tp::PendingOperation*);
		void handleRemoved (Tp::PendingOperation*);
		void handleRenamed (Tp::PendingOperation*);

		void handleConnStatusChanged (Tp::ConnectionStatus);
		void handleConnectionChanged (Tp::ConnectionPtr);

		void handlePasswordFixed (Tp::PendingOperation*);
		void handleRequestedPresenceFinish (Tp::PendingOperation*);
		void handleCurrentPresence (Tp::Presence);

		void handleAccountAvatar (const Tp::Avatar&);

		void handlePresencePubRequested (Tp::Contacts);
		void handleCMStateChanged (Tp::ContactListState);
		void handleKnownContactsChanged (Tp::Contacts,
				Tp::Contacts, Tp::Channel::GroupMemberChangeDetails);

		void handleAuthRequestFinished (Tp::PendingOperation*);
		void handleAuthRequestSent (Tp::PendingOperation*);
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void accountRenamed (const QString&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&, const QString&, const QString&);

		void removeFinished (AccountWrapper*);

		void gotEntity (LC::Entity);
	};
}
}
}
