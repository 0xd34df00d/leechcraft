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

#include "accountsmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/istorageaccount.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	AccountsManager::AccountsManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Account")) << tr ("Storage"));
	}

	void AccountsManager::AddPlugin (IStoragePlugin *plug)
	{
		Plugins_ << plug;

		Q_FOREACH (QObject *acc, plug->GetAccounts ())
			handleAccountAdded (acc);

		connect (plug->GetObject (),
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (handleAccountAdded (QObject*)));
		connect (plug->GetObject (),
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)));
	}

	QList<IStoragePlugin*> AccountsManager::GetPlugins () const
	{
		return Plugins_;
	}

	QList<IStorageAccount*> AccountsManager::GetAccounts () const
	{
		QList<IStorageAccount*> accounts;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto data = Model_->item (i)->data (Roles::AccountObj);
			accounts << qobject_cast<IStorageAccount*> (data.value<QObject*> ());
		}
		return accounts;
	}

	QAbstractItemModel* AccountsManager::GetModel () const
	{
		return Model_;
	}

	void AccountsManager::RemoveAccount (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		QObject *accObj = index.sibling (index.row (), 0)
				.data (Roles::AccountObj).value<QObject*> ();
		auto acc = qobject_cast<IStorageAccount*> (accObj);

		auto plugin = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
		plugin->RemoveAccount (accObj);
	}

	void AccountsManager::handleAccountAdded (QObject *obj)
	{
		IStorageAccount *acc = qobject_cast<IStorageAccount*> (obj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj;
			return;
		}

		QObject *pObj = acc->GetParentPlugin ();
		IStoragePlugin *plugin = qobject_cast<IStoragePlugin*> (pObj);

		QList<QStandardItem*> row;
		row << new QStandardItem (plugin->GetStorageIcon (), acc->GetAccountName ());
		row << new QStandardItem (plugin->GetStorageName ());
		Model_->appendRow (row);

		row.first ()->setData (QVariant::fromValue<QObject*> (obj), Roles::AccountObj);
	}

	void AccountsManager::handleAccountRemoved (QObject *obj)
	{
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			if (Model_->item (i)->data (AccountObj).value<QObject*> () == obj)
				continue;

			Model_->removeRow (i);
			break;
		}
	}
}
}
