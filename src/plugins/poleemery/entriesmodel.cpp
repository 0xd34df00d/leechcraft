/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entriesmodel.h"
#include <QColor>
#include <QApplication>
#include <QFontMetrics>
#include <util/sll/typegetter.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "accountsmanager.h"
#include "operationsmanager.h"
#include "currenciesmanager.h"

namespace LC
{
namespace Poleemery
{
	EntriesModel::EntriesModel (QObject *parent)
	: QAbstractItemModel (parent)
	, HeaderData_ {
			tr ("Date"),
			tr ("Name"),
			tr ("Price"),
			tr ("Currency"),
			tr ("Rate"),
			tr ("Native price"),
			tr ("Count"),
			tr ("Shop"),
			tr ("Categories"),
			tr ("Account"),
			tr ("Account balance"),
			tr ("Sum balance")
		}
	{
	}

	QModelIndex EntriesModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (parent.isValid ())
			return QModelIndex ();

		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex EntriesModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int EntriesModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Entries_.size ();
	}

	int EntriesModel::columnCount (const QModelIndex&) const
	{
		return HeaderData_.size ();
	}

	Qt::ItemFlags EntriesModel::flags (const QModelIndex& index) const
	{
		auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		switch (index.column ())
		{
		case Columns::AccBalance:
		case Columns::SumBalance:
			break;
		case Columns::Count:
		case Columns::Shop:
		case Columns::Categories:
			if (Entries_.at (index.row ())->GetType () == EntryType::Expense)
				flags |= Qt::ItemIsEditable;
			break;
		case Columns::EntryCurrency:
		case Columns::EntryRate:
		case Columns::NativePrice:
			if (RatePriceEditable_ &&
					Entries_.at (index.row ())->GetType () == EntryType::Expense)
				flags |= Qt::ItemIsEditable;
			else
				flags &= ~Qt::ItemIsEnabled;
			break;
		default:
			flags |= Qt::ItemIsEditable;
			break;
		}
		return flags;
	}

	namespace
	{
		template<typename Getter>
		QVariant GetDataIf (EntryBase_ptr entry, EntryType expected,
				Getter getter,
				const QVariant& def = QVariant ())
		{
			if (entry->GetType () != expected)
				return def;

			using T = typename std::decay_t<Util::ArgType_t<Getter, 0>>::element_type;

			auto derived = std::dynamic_pointer_cast<T> (entry);
			return getter (derived);
		}
	}

	QVariant EntriesModel::data (const QModelIndex& index, int role) const
	{
		auto entry = Entries_.at (index.row ());

		switch (role)
		{
		case Qt::DisplayRole:
		{
			auto accsManager = Core::Instance ().GetAccsManager ();
			auto acc = accsManager->GetAccount (entry->AccountID_);

			switch (index.column ())
			{
			case Columns::Account:
				return acc.Name_;
			case Columns::Name:
				return entry->Name_;
			case Columns::Price:
				return QString::number (entry->Amount_, 'f', 2);
			case Columns::EntryCurrency:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->EntryCurrency_; });
			case Columns::EntryRate:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return QString::number (exp->Rate_, 'f', 3); });
			case Columns::NativePrice:
				return GetDataIf (entry, EntryType::Expense,
						[&acc] (ExpenseEntry_ptr exp)
						{
							return QString::number (exp->Rate_ * exp->Amount_, 'f', 2) +
									" " + acc.Currency_;
						});
			case Columns::Date:
				return entry->Date_;
			case Columns::Count:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->Count_; });
			case Columns::Shop:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->Shop_; });
			case Columns::Categories:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) -> QVariant
						{
							auto itm = Core::Instance ().GetCoreProxy ()->GetTagsManager ();
							return itm->Join (exp->Categories_);
						});
			case Columns::AccBalance:
				return QString::number (Sums_ [index.row ()].Accs_ [entry->AccountID_]) + " " + acc.Currency_;
			case Columns::SumBalance:
				return QString::number (Sums_ [index.row ()].Total_) + " " +
						Core::Instance ().GetCurrenciesManager ()->GetUserCurrency ();
			}
			break;
		}
		case Qt::EditRole:
		{
			auto acc = Core::Instance ().GetAccsManager ()->GetAccount (entry->AccountID_);

			switch (index.column ())
			{
			case Columns::Account:
				return static_cast<int> (acc.ID_);
			case Columns::Name:
				return entry->Name_;
			case Columns::Price:
				return entry->Amount_;
			case Columns::EntryCurrency:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->EntryCurrency_; });
			case Columns::EntryRate:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->Rate_; });
			case Columns::NativePrice:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->Rate_ * exp->Amount_; });
			case Columns::Date:
				return entry->Date_;
			case Columns::Count:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->Count_; });
			case Columns::Shop:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->Shop_; });
			case Columns::Categories:
				return GetDataIf (entry, EntryType::Expense,
						[] (ExpenseEntry_ptr exp) { return exp->Categories_; });
			case Columns::AccBalance:
				return QVariant ();
			case Columns::SumBalance:
				return QVariant ();
			}
			break;
		}
		case Qt::BackgroundRole:
			switch (entry->GetType ())
			{
			case EntryType::Expense:
				return QColor (127, 0, 0, 32);
			case EntryType::Receipt:
				return QColor (0, 127, 0, 32);
			}
			break;
		}

		return QVariant ();
	}

	bool EntriesModel::setData (const QModelIndex& index, const QVariant& value, int)
	{
		auto entry = Entries_.at (index.row ());
		bool shouldRecalcSums = false;
		bool resetModel = false;

		auto expEntry = entry->GetType () == EntryType::Expense ?
				std::dynamic_pointer_cast<ExpenseEntry> (entry) :
				ExpenseEntry_ptr ();

		switch (index.column ())
		{
		case Columns::Account:
			entry->AccountID_ = value.toInt ();
			shouldRecalcSums = true;
			break;
		case Columns::Name:
			entry->Name_ = value.toString ();
			break;
		case Columns::Price:
			entry->Amount_ = value.toDouble ();
			shouldRecalcSums = true;
			break;
		case Columns::EntryCurrency:
			expEntry->EntryCurrency_ = value.toString ();
			shouldRecalcSums = true;
			break;
		case Columns::EntryRate:
			expEntry->Rate_ = value.toDouble ();
			shouldRecalcSums = true;
			break;
		case Columns::NativePrice:
			if (expEntry->Amount_)
			{
				expEntry->Rate_ = value.toDouble () / expEntry->Amount_;
				shouldRecalcSums = true;
			}
			break;
		case Columns::Date:
			entry->Date_ = value.toDateTime ();
			resetModel = true;
			break;
		case Columns::Count:
			expEntry->Count_ = value.toDouble ();
			break;
		case Columns::Shop:
			expEntry->Shop_ = value.toString ();
			break;
		case Columns::Categories:
			expEntry->Categories_ = value.toStringList ();
			break;
		}

		if (ModifiesStorage_)
			Core::Instance ().GetOpsManager ()->UpdateEntry (entry);

		if (resetModel)
		{
			auto entries = Entries_;
			Entries_.clear ();
			AddEntries (entries);
		}
		else
		{
			emit dataChanged (createIndex (index.row (), 0), createIndex (index.row (), Columns::MaxCount));
			if (shouldRecalcSums)
				recalcSums ();
		}

		return true;
	}

	QVariant EntriesModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Vertical)
			return QVariant ();

		switch (role)
		{
		case Qt::DisplayRole:
			return HeaderData_.at (section);
		}

		return QVariant ();
	}

	void EntriesModel::SetRatePriceEditable (bool editable)
	{
		RatePriceEditable_ = editable;
	}

	void EntriesModel::SetModifiesStorage (bool modifies)
	{
		ModifiesStorage_ = modifies;
	}

	namespace
	{
		bool DateLess (EntryBase_ptr e1, EntryBase_ptr e2)
		{
			return e1->Date_ < e2->Date_;
		}

		void AppendEntry (QHash<int, double>& hash, EntryBase_ptr entry)
		{
			auto& val = hash [entry->AccountID_];
			switch (entry->GetType ())
			{
			case EntryType::Expense:
			{
				const auto expEntry = std::dynamic_pointer_cast<ExpenseEntry> (entry);
				val -= expEntry->Amount_ * expEntry->Rate_;
				break;
			}
			case EntryType::Receipt:
				val += entry->Amount_;
				break;
			}
		}

		double GetTotalSum (const QHash<int, double>& curSum, AccountsManager *accMgr, CurrenciesManager *curMgr)
		{
			double totalSum = 0;
			for (auto id : curSum.keys ())
			{
				auto acc = accMgr->GetAccount (id);
				totalSum += curMgr->ToUserCurrency (acc.Currency_, curSum [id]);
			}
			return totalSum;
		}
	}

	void EntriesModel::AddEntry (EntryBase_ptr entry)
	{
		auto bound = std::upper_bound (Entries_.begin (), Entries_.end (), entry, DateLess);

		auto pos = std::distance (Entries_.begin (), bound);
		beginInsertRows (QModelIndex (), pos, pos);

		Entries_.insert (bound, entry);
		if (bound == Entries_.end ())
		{
			auto hash = Sums_.value (Sums_.size () - 1).Accs_;
			AppendEntry (hash, entry);
			const auto totalSum = GetTotalSum (hash,
					Core::Instance ().GetAccsManager (),
					Core::Instance ().GetCurrenciesManager ());
			Sums_ << BalanceInfo { totalSum, hash };
		}
		else
			recalcSums ();

		endInsertRows ();
	}

	void EntriesModel::AddEntries (QList<EntryBase_ptr> entries)
	{
		beginResetModel ();

		Entries_ += entries;
		std::sort (Entries_.begin (), Entries_.end (), DateLess);

		recalcSums ();

		endResetModel ();
	}

	void EntriesModel::RemoveEntry (const QModelIndex& index)
	{
		beginRemoveRows (QModelIndex (), index.row (), index.row ());
		Entries_.removeAt (index.row ());
		endRemoveRows ();

		recalcSums ();
	}

	EntryBase_ptr EntriesModel::GetEntry (const QModelIndex& index) const
	{
		return Entries_.value (index.row ());
	}

	QList<EntryBase_ptr> EntriesModel::GetEntries () const
	{
		return Entries_;
	}

	QList<BalanceInfo> EntriesModel::GetSumInfos () const
	{
		return Sums_;
	}

	void EntriesModel::recalcSums ()
	{
		Sums_.clear ();

		if (Entries_.isEmpty ())
			return;

		auto accMgr = Core::Instance ().GetAccsManager ();
		auto curMgr = Core::Instance ().GetCurrenciesManager ();

		QHash<int, double> curSum;
		for (auto entry : Entries_)
		{
			AppendEntry (curSum, entry);
			const auto totalSum = GetTotalSum (curSum, accMgr, curMgr);
			Sums_ << BalanceInfo { totalSum, curSum };
		}

		emit dataChanged (createIndex (0, Columns::AccBalance),
				createIndex (Entries_.size () - 1, Columns::SumBalance));
	}
}
}
