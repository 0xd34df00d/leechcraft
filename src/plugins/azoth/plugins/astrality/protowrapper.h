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
#include <Types>
#include <ConnectionManager>
#include <AccountManager>
#include <interfaces/iprotocol.h>
#include <interfaces/structures.h>
#include "accountwrapper.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	class AccountWrapper;

	class ProtoWrapper : public QObject
					   , public IProtocol
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IProtocol);

		Tp::ConnectionManagerPtr CM_;
		QString ProtoName_;
		const Tp::ProtocolInfo ProtoInfo_;

		Tp::AccountManagerPtr AM_;

		QList<AccountWrapper*> Accounts_;
		QMap<Tp::PendingAccount*, AccountWrapper::Settings> PendingSettings_;
	public:
		ProtoWrapper (Tp::ConnectionManagerPtr,
				const QString&, QObject*);

		QVariantMap GetParamsFromWidgets (const QList<QWidget*>&) const;
		AccountWrapper::Settings GetSettingsFromWidgets (const QList<QWidget*>&) const;

		QObject* GetObject ();
		ProtocolFeatures GetFeatures () const;
		QList<QObject*> GetRegisteredAccounts ();
		QObject* GetParentProtocolPlugin () const;
		QString GetProtocolName () const;
		QIcon GetProtocolIcon () const;
		QByteArray GetProtocolID () const;
		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions);
		void RegisterAccount (const QString&, const QList<QWidget*>&);
		QWidget* GetMUCJoinWidget ();
		void RemoveAccount (QObject*);
	private slots:
		void handleAMReady (Tp::PendingOperation*);
		void handleAccountCreated (Tp::PendingOperation*);
		AccountWrapper* handleNewAccount (Tp::AccountPtr);
		void handleAccountRemoved (AccountWrapper*);
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);

		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (LeechCraft::Entity, int*, QObject**);
	};
}
}
}
