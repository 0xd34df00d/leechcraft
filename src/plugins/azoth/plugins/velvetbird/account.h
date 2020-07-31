/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <libpurple/account.h>
#include <interfaces/azoth/iaccount.h>
#include "protocol.h"

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	class Protocol;
	class Buddy;

	class Account : public QObject
				  , public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAccount)

		PurpleAccount * const Account_;
		Protocol * const Proto_;

		EntryStatus CurrentStatus_;

		QHash<PurpleBuddy*, Buddy*> Buddies_;
	public:
		Account (PurpleAccount*, Protocol*);

		void Release ();

		PurpleAccount* GetPurpleAcc () const;

		QObject* GetQObject ();
		Protocol* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		void UpdateBuddy (PurpleBuddy*);
		void RemoveBuddy (PurpleBuddy*);

		void HandleDisconnect (PurpleConnectionError, const char*);
		void HandleConvLessMessage (PurpleConversation*,
			const char*, const char*, PurpleMessageFlags, time_t);
		void UpdateStatus ();
		void HandleStatus (PurpleStatus*);
	public slots:
		void updateIcon ();
	private slots:
		void handleAuthFailure (const LC::Azoth::EntryStatus&);
	signals:
		void accountRenamed (const QString&);
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&, const QString&, const QString&);
	};
}
}
}
