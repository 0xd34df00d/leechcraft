/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include "structures.h"

namespace LC
{
namespace Poleemery
{
	class Storage;
	typedef std::shared_ptr<Storage> Storage_ptr;

	class AccountsManager : public QObject
	{
		Q_OBJECT

		const Storage_ptr Storage_;
		QHash<int, Account> Accounts_;
	public:
		AccountsManager (Storage_ptr, QObject* = 0);

		QList<Account> GetAccounts () const;
		Account GetAccount (int) const;

		void AddAccount (Account);
		void UpdateAccount (const Account&);
		void DeleteAccount (const Account&);
	};
}
}
