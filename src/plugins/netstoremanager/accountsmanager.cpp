/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

	QAbstractItemModel* AccountsManager::GetModel () const
	{
		return Model_;
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

		row.first ()->setData (QVariant::fromValue<QObject*> (obj), AccountObj);
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
