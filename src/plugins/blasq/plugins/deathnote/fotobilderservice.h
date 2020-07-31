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
#include <interfaces/structures.h>

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	class FotoBilderAccount;

	class FotoBilderService : public QObject
							, public IService
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IService)

		const ICoreProxy_ptr Proxy_;
		QList<FotoBilderAccount*> Accounts_;
	public:
		FotoBilderService (ICoreProxy_ptr proxy);

		QObject* GetQObject ();

		QString GetServiceName () const;
		QIcon GetServiceIcon () const;

		QList<IAccount*> GetRegisteredAccounts () const;
		QList<QWidget*> GetAccountRegistrationWidgets () const;
		void RegisterAccount (const QString& name, const QList<QWidget*>& widgets);
		void RemoveAccount (IAccount *account);
	private:
		void AddAccount (FotoBilderAccount *account);
		void ReadAccounts ();

	private slots:
		void saveAccount (FotoBilderAccount *account);
	signals:
		void accountAdded (QObject *accObj);
		void accountRemoved (QObject *accObj);
	};
}
}
}
