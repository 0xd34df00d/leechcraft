/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountsmanager.h"
#include "storage.h"

namespace LC
{
namespace Poleemery
{
	AccountsManager::AccountsManager (Storage_ptr storage, QObject *parent)
	: QObject (parent)
	, Storage_ (storage)
	{
		for (const auto& acc : Storage_->GetAccounts ())
			Accounts_ [acc.ID_] = acc;
	}

	QList<Account> AccountsManager::GetAccounts () const
	{
		return Accounts_.values ();
	}

	Account AccountsManager::GetAccount (int id) const
	{
		return Accounts_ [id];
	}

	void AccountsManager::AddAccount (Account acc)
	{
		Storage_->AddAccount (acc);
		Accounts_ [acc.ID_] = acc;
	}

	void AccountsManager::UpdateAccount (const Account& acc)
	{
		Storage_->UpdateAccount (acc);
		Accounts_ [acc.ID_] = acc;
	}

	void AccountsManager::DeleteAccount (const Account& acc)
	{
		Storage_->DeleteAccount (acc);
		Accounts_.remove (acc.ID_);
	}
}
}
