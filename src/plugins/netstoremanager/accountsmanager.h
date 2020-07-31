/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_ACCOUNTSMANAGER_H
#define PLUGINS_NETSTOREMANAGER_ACCOUNTSMANAGER_H
#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/netstoremanager/isupportfilelistings.h"

class QAbstractItemModel;
class QStandardItemModel;
class QModelIndex;

namespace LC
{
namespace NetStoreManager
{
	class IStoragePlugin;
	class IStorageAccount;

	class AccountsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		enum Roles
		{
			AccountObj = Qt::UserRole + 1
		};

		QList<IStoragePlugin*> Plugins_;
		ICoreProxy_ptr Proxy_;

	public:
		AccountsManager (ICoreProxy_ptr proxy, QObject* = 0);

		void AddPlugin (IStoragePlugin*);
		QList<IStoragePlugin*> GetPlugins () const;
		QList<IStorageAccount*> GetAccounts () const;
		IStorageAccount* GetAccountFromUniqueID (const QString& id) const;
		QAbstractItemModel* GetModel () const;

		void RemoveAccount (const QModelIndex&);

		ICoreProxy_ptr GetProxy () const;

	private slots:
		void handleAccountAdded (QObject*);
		void handleAccountRemoved (QObject*);

	signals:
		void accountAdded (QObject *acc);
		void accountRemoved (QObject *acc);
	};
}
}

#endif
