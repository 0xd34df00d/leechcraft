/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountsmanager.h"
#include <QStandardItemModel>
#include "interfaces/blasq/iservice.h"
#include "interfaces/blasq/iaccount.h"
#include "servicesmanager.h"

namespace LC
{
namespace Blasq
{
	AccountsManager::AccountsManager (ServicesManager *mgr, QObject *parent)
	: QObject (parent)
	, SvcMgr_ (mgr)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Service") });

		connect (mgr,
				SIGNAL (serviceAdded (IService*)),
				this,
				SLOT (handleService (IService*)));
		for (auto service : mgr->GetServices ())
			handleService (service);
	}

	QAbstractItemModel* AccountsManager::GetModel ()
	{
		return Model_;
	}

	const QList<IAccount*>& AccountsManager::GetAccounts () const
	{
		return Accounts_;
	}

	int AccountsManager::GetAccountIndex (const QByteArray& id) const
	{
		const auto pos = std::find_if (Accounts_.begin (), Accounts_.end (),
				[&id] (IAccount *acc) { return acc->GetID () == id; });
		return pos == Accounts_.end () ?
				-1 :
				static_cast<int> (std::distance (Accounts_.begin (), pos));
	}

	IAccount* AccountsManager::GetAccount (const QByteArray& id) const
	{
		const auto pos = std::find_if (Accounts_.begin (), Accounts_.end (),
				[&id] (IAccount *acc) { return acc->GetID () == id; });
		return pos == Accounts_.end () ? nullptr : *pos;
	}

	void AccountsManager::RemoveAccount (const QModelIndex& index)
	{
		const auto accObj = index.data (Role::AccountObj).value<QObject*> ();
		const auto acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
			return;

		const auto service = acc->GetService ();
		service->RemoveAccount (acc);
	}

	void AccountsManager::HandleAccount (IAccount *acc)
	{
		auto service = acc->GetService ();

		auto row =
		{
			new QStandardItem (service->GetServiceIcon (), acc->GetName ()),
			new QStandardItem (service->GetServiceName ())
		};

		auto accVar = QVariant::fromValue (acc->GetQObject ());
		for (auto item : row)
		{
			item->setEditable (false);
			item->setData (accVar, Role::AccountObj);
			item->setData (acc->GetID (), Role::AccountId);
		}
		Model_->appendRow (row);

		Accounts_ << acc;
	}

	void AccountsManager::handleService (IService *service)
	{
		const auto& accs = service->GetRegisteredAccounts ();
		for (auto acc : accs)
			HandleAccount (acc);

		auto serviceObj = service->GetQObject ();
		connect (serviceObj,
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (handleAccountAdded (QObject*)));
		connect (serviceObj,
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)));
	}

	void AccountsManager::handleAccountAdded (QObject *accObj)
	{
		HandleAccount (qobject_cast<IAccount*> (accObj));
	}

	void AccountsManager::handleAccountRemoved (QObject *accObj)
	{
		Accounts_.removeAll (qobject_cast<IAccount*> (accObj));

		auto accVar = QVariant::fromValue (accObj);

		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			if (Model_->item (i)->data (Role::AccountObj) != accVar)
				continue;

			Model_->removeRow (i);
			break;
		}
	}
}
}
