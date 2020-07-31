/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/blasq/iservice.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	class AuthManager;
	class PicasaAccount;

	class PicasaService : public QObject
					, public IService
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IService)

		const ICoreProxy_ptr Proxy_;

		QList<PicasaAccount*> Accounts_;
		AuthManager *AuthManager_;

	public:
		PicasaService (ICoreProxy_ptr proxy);

		QObject* GetQObject ();

		QString GetServiceName () const;
		QIcon GetServiceIcon () const;

		void Release ();

		QList<IAccount*> GetRegisteredAccounts () const;
		QList<QWidget*> GetAccountRegistrationWidgets () const;
		void RegisterAccount (const QString& name, const QList<QWidget*>& widgets);
		void RemoveAccount (IAccount *account);
	private:
		void AddAccount (PicasaAccount *account);
		void ReadAccounts ();

	private slots:
		void saveAccount (PicasaAccount *account);
		void handleAuthSuccess (QObject *accObj);

	signals:
		void accountAdded (QObject *accObj);
		void accountRemoved (QObject *accObj);
	};
}
}
}
