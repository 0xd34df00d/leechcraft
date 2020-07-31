/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <Types>
#include <ConnectionManager>
#include <AccountManager>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iprotocol.h>
#include "accountwrapper.h"

namespace LC
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
		Q_INTERFACES (LC::Azoth::IProtocol);

		Tp::ConnectionManagerPtr CM_;
		const QString ProtoName_;
		const ICoreProxy_ptr Proxy_;
		const Tp::ProtocolInfo ProtoInfo_;

		Tp::AccountManagerPtr AM_;

		QList<AccountWrapper*> Accounts_;
		QMap<Tp::PendingAccount*, AccountWrapper::Settings> PendingSettings_;
	public:
		ProtoWrapper (Tp::ConnectionManagerPtr, const QString&, const ICoreProxy_ptr&, QObject*);

		void Release ();

		QVariantMap GetParamsFromWidgets (const QList<QWidget*>&) const;
		AccountWrapper::Settings GetSettingsFromWidgets (const QList<QWidget*>&) const;

		QObject* GetQObject ();
		ProtocolFeatures GetFeatures () const;
		QList<QObject*> GetRegisteredAccounts ();
		QObject* GetParentProtocolPlugin () const;
		QString GetProtocolName () const;
		QIcon GetProtocolIcon () const;
		QByteArray GetProtocolID () const;
		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions);
		void RegisterAccount (const QString&, const QList<QWidget*>&);
		void RemoveAccount (QObject*);
	private slots:
		void handleAMReady (Tp::PendingOperation*);
		void handleAccountCreated (Tp::PendingOperation*);
		AccountWrapper* handleNewAccount (Tp::AccountPtr);
		void handleAccountRemoved (AccountWrapper*);
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);

		void gotEntity (const LC::Entity&);
	};
}
}
}
