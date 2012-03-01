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

#ifndef PLUGINS_NETSTOREMANAGER_ACCOUNTSMANAGER_H
#define PLUGINS_NETSTOREMANAGER_ACCOUNTSMANAGER_H
#include <QObject>

class QAbstractItemModel;
class QStandardItemModel;
class QModelIndex;

namespace LeechCraft
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
	public:
		AccountsManager (QObject* = 0);

		void AddPlugin (IStoragePlugin*);
		QList<IStoragePlugin*> GetPlugins () const;
		QList<IStorageAccount*> GetAccounts () const;
		QAbstractItemModel* GetModel () const;

		void RemoveAccount (const QModelIndex&);
	private slots:
		void handleAccountAdded (QObject*);
		void handleAccountRemoved (QObject*);
	};
}
}

#endif
