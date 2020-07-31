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
#include <QList>
#include "structures.h"

namespace LC
{
namespace Poleemery
{
	struct StorageImpl;

	class Storage : public QObject
	{
		std::shared_ptr<StorageImpl> Impl_;
	public:
		Storage (QObject* = 0);

		Storage (const Storage&) = delete;
		Storage (Storage&&) = delete;
		Storage& operator= (const Storage&) = delete;
		Storage& operator= (Storage&&) = delete;

		QList<Account> GetAccounts () const;
		void AddAccount (Account&);
		void UpdateAccount (const Account&);
		void DeleteAccount (const Account&);

		QList<ExpenseEntry> GetExpenseEntries ();
		QList<ExpenseEntry> GetExpenseEntries (const Account&);
		void AddExpenseEntry (ExpenseEntry&);
		void UpdateExpenseEntry (const ExpenseEntry&);
		void DeleteExpenseEntry (const ExpenseEntry&);

		QList<ReceiptEntry> GetReceiptEntries ();
		QList<ReceiptEntry> GetReceiptEntries (const Account&);
		void AddReceiptEntry (ReceiptEntry&);
		void UpdateReceiptEntry (const ReceiptEntry&);
		void DeleteReceiptEntry (const ReceiptEntry&);

		QList<Rate> GetRates ();
		QList<Rate> GetRates (const QDateTime& start, const QDateTime& end);
		QList<Rate> GetRate (const QString&);
		QList<Rate> GetRate (const QString&, const QDateTime& start, const QDateTime& end);
		void AddRate (Rate&);
	private:
		Category AddCategory (const QString&);
		void AddNewCategories (const ExpenseEntry&, const QStringList&);
		void LinkEntry2Cat (const ExpenseEntry&, const Category&);
		void UnlinkEntry2Cat (const ExpenseEntry&, const Category&);

		QList<ExpenseEntry> HandleNaked (const QList<NakedExpenseEntry>&);

		void InitializeTables ();
		void LoadCategories ();
	};
}
}
