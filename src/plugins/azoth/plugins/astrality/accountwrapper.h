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

#include <TelepathyQt/Types>
#include <TelepathyQt/Account>
#include <interfaces/structures.h>
#include <interfaces/iaccount.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	class AccountWrapper : public QObject
						 , public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount);

		Tp::AccountPtr A_;
	public:
		AccountWrapper (Tp::AccountPtr, QObject*);

		QObject* GetObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString&);
		QByteArray GetAccountID () const;
		QList<QAction*> GetActions () const;
		void QueryInfo (const QString&);
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager() const;
	private:
		void HandleAuth (bool failure);
	private slots:
		void handleEnabled (Tp::PendingOperation*);
		void handleConnStatusChanged (Tp::ConnectionStatus);
		void handlePasswordFixed (Tp::PendingOperation*);
		void handleRequestedPresenceFinish (Tp::PendingOperation*);
		void handleCurrentPresence (Tp::Presence);
	signals:
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

		void gotEntity (LeechCraft::Entity);
		void delegateEntity (LeechCraft::Entity, int*, QObject**);
	};
}
}
}
