/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "account.h"

class QAbstractItemModel;
class QStandardItemModel;
class QModelIndex;

namespace LC
{
namespace Snails
{
	class ProgressManager;
	class Storage;

	class AccountsManager : public QObject
	{
		QStandardItemModel * const AccountsModel_;
		QList<Account_ptr> Accounts_;

		ProgressManager * const ProgressMgr_;
		Storage * const Storage_;
	public:
		AccountsManager (ProgressManager*, Storage*, QObject* = nullptr);

		QAbstractItemModel* GetAccountsModel () const;
		QList<Account_ptr> GetAccounts () const;
		Account_ptr GetAccount (const QModelIndex&) const;

		void InitiateAccountAddition ();

		void InitWithPlugins ();
	private:
		void AddAccountImpl (Account_ptr);
		void LoadAccounts ();
		void SaveAccounts () const;
	};
}
}
