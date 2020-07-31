/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "operationsmanager.h"
#include <QStandardItemModel>
#include <util/sll/prelude.h>
#include "storage.h"
#include "entriesmodel.h"
#include "core.h"
#include "currenciesmanager.h"

namespace LC
{
namespace Poleemery
{
	OperationsManager::OperationsManager (Storage_ptr storage, QObject *parent)
	: QObject (parent)
	, Storage_ (storage)
	, Model_ (new EntriesModel (this))
	{
	}

	void OperationsManager::Load ()
	{
		QList<EntryBase_ptr> entries;
		for (const auto& entry : Storage_->GetReceiptEntries ())
			entries << std::make_shared<ReceiptEntry> (entry);
		for (const auto& entry : Storage_->GetExpenseEntries ())
		{
			entries << std::make_shared<ExpenseEntry> (entry);
			for (const auto& cat : entry.Categories_)
				KnownCategories_ << cat;
		}
		for (auto entry : entries)
		{
			auto& date = entry->Date_;
			const auto& time = date.time ();
			date.setTime ({ time.hour (), time.minute () });
		}
		Model_->AddEntries (entries);

		connect (Core::Instance ().GetCurrenciesManager (),
				SIGNAL (currenciesUpdated ()),
				Model_,
				SLOT (recalcSums ()));
	}

	QAbstractItemModel* OperationsManager::GetModel () const
	{
		return Model_;
	}

	QList<EntryBase_ptr> OperationsManager::GetAllEntries () const
	{
		return Model_->GetEntries ();
	}

	QList<EntryWithBalance> OperationsManager::GetEntriesWBalance () const
	{
		return Util::ZipWith (Model_->GetEntries (), Model_->GetSumInfos (),
					[] (EntryBase_ptr e, BalanceInfo i) { return EntryWithBalance { e, i }; });
	}

	QSet<QString> OperationsManager::GetKnownCategories () const
	{
		return KnownCategories_;
	}

	void OperationsManager::AddEntry (EntryBase_ptr entry)
	{
		switch (entry->GetType ())
		{
		case EntryType::Expense:
		{
			auto expense = std::dynamic_pointer_cast<ExpenseEntry> (entry);
			Storage_->AddExpenseEntry (*expense);
			for (const auto& cat : expense->Categories_)
				KnownCategories_ << cat;
			break;
		}
		case EntryType::Receipt:
			Storage_->AddReceiptEntry (*std::dynamic_pointer_cast<ReceiptEntry> (entry));
			break;
		}

		Model_->AddEntry (entry);
	}

	void OperationsManager::UpdateEntry (EntryBase_ptr entry)
	{
		switch (entry->GetType ())
		{
		case EntryType::Expense:
		{
			auto expense = std::dynamic_pointer_cast<ExpenseEntry> (entry);
			Storage_->UpdateExpenseEntry (*expense);
			for (const auto& cat : expense->Categories_)
				KnownCategories_ << cat;
			break;
		}
		case EntryType::Receipt:
			Storage_->UpdateReceiptEntry (*std::dynamic_pointer_cast<ReceiptEntry> (entry));
			break;
		}
	}

	void OperationsManager::RemoveEntry (const QModelIndex& index)
	{
		auto entry = Model_->GetEntry (index);

		switch (entry->GetType ())
		{
		case EntryType::Expense:
			Storage_->DeleteExpenseEntry (*std::dynamic_pointer_cast<ExpenseEntry> (entry));
			break;
		case EntryType::Receipt:
			Storage_->DeleteReceiptEntry (*std::dynamic_pointer_cast<ReceiptEntry> (entry));
			break;
		}

		Model_->RemoveEntry (index);
	}
}
}
