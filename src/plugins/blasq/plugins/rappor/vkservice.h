/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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
namespace Rappor
{
	class VkAccount;

	class VkService : public QObject
					, public IService
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IService)

		const ICoreProxy_ptr Proxy_;

		QList<VkAccount*> Accounts_;
	public:
		VkService (ICoreProxy_ptr);

		QObject* GetQObject ();

		QString GetServiceName () const;
		QIcon GetServiceIcon () const;

		QList<IAccount*> GetRegisteredAccounts () const;
		QList<QWidget*> GetAccountRegistrationWidgets () const;
		void RegisterAccount (const QString&, const QList<QWidget*>&);
		void RemoveAccount (IAccount*);
	private:
		void AddAccount (VkAccount*);
	private slots:
		void saveAccount (VkAccount*);
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
}
}
}
