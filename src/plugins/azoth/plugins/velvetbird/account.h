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
#include <libpurple/account.h>
#include <interfaces/azoth/iaccount.h>

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	class Protocol;

	class Account : public QObject
				  , public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount)

		QString Name_;
		PurpleAccount *Account_;
		Protocol *Proto_;
	public:
		Account (const QString&, PurpleAccount*, Protocol*);

		PurpleAccount* GetPurpleAcc () const;

		QObject* GetObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		QString GetAccountName () const;
		QString GetOurNick () const;
		void RenameAccount (const QString& name);
		virtual QByteArray GetAccountID () const;
		QList< QAction* > GetActions () const;
		void QueryInfo (const QString& address);
		void OpenConfigurationDialog ();
		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&, const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		void HandleStatus (PurpleStatus*);
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
