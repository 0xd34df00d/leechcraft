/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStringList>
#include <QDateTime>
#include <QMetaType>
#include <QHash>
#include <util/db/oral/oraltypes.h>

namespace LC
{
namespace Poleemery
{
	enum class AccType
	{
		Cash,
		BankAccount
	};

	QString ToHumanReadable (AccType);

	struct Account
	{
		Util::oral::PKey<int> ID_;
		AccType Type_;
		QString Name_;
		QString Currency_;

		static QString ClassName () { return "Account"; }

		static QString FieldNameMorpher (const QString& str) { return str; }
	};

	bool operator== (const Account&, const Account&);
	bool operator!= (const Account&, const Account&);
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Poleemery::Account,
		ID_,
		Type_,
		Name_,
		Currency_)

Q_DECLARE_METATYPE (LC::Poleemery::Account)

namespace LC
{
namespace Poleemery
{
	enum class EntryType
	{
		Expense,
		Receipt
	};

	struct EntryBase
	{
		Util::oral::PKey<int> ID_ = -1;
		Util::oral::References<&Account::ID_> AccountID_ = -1;

		/** This actually price.
		 */
		double Amount_ = 0;
		QString Name_;
		QString Description_;
		QDateTime Date_;

		EntryBase () = default;
		EntryBase (int accId, double amount, const QString& name, const QString& descr, const QDateTime& dt);
		virtual ~EntryBase () = default;

		virtual EntryType GetType () const = 0;
	};

	typedef std::shared_ptr<EntryBase> EntryBase_ptr;

	struct NakedExpenseEntry : EntryBase
	{
		double Count_ = 0;
		QString Shop_;

		QString EntryCurrency_;
		double Rate_ = 0;

		static QString ClassName () { return "NakedExpenseEntry"; }

		static QString FieldNameMorpher (const QString& str) { return str; }

		EntryType GetType () const { return EntryType::Expense; }

		NakedExpenseEntry () = default;
		NakedExpenseEntry (int accId, double amount,
				const QString& name, const QString& descr, const QDateTime& dt,
				double count, const QString& shop, const QString& currency, double rate);
	};

	struct ExpenseEntry : NakedExpenseEntry
	{
		QStringList Categories_;

		ExpenseEntry () = default;
		ExpenseEntry (const NakedExpenseEntry&);
		ExpenseEntry (int accId, double amount,
				const QString& name, const QString& descr, const QDateTime& dt,
				double count, const QString& shop, const QString& currency, double rate,
				const QStringList& cats);
	};

	typedef std::shared_ptr<ExpenseEntry> ExpenseEntry_ptr;
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Poleemery::NakedExpenseEntry,
		ID_,
		AccountID_,
		Amount_,
		Name_,
		Description_,
		Date_,
		Count_,
		Shop_,
		EntryCurrency_,
		Rate_)

namespace LC
{
namespace Poleemery
{
	struct Category
	{
		Util::oral::PKey<int> ID_ = -1;
		Util::oral::Unique<QString> Name_;

		Category () = default;
		explicit Category (const QString&);

		static QString ClassName () { return "Category"; }

		static QString FieldNameMorpher (const QString& str) { return str; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Poleemery::Category,
		ID_,
		Name_)

namespace LC
{
namespace Poleemery
{
	struct CategoryLink
	{
		Util::oral::PKey<int> ID_ = -1;
		Util::oral::References<&Category::ID_> Category_;
		Util::oral::References<&NakedExpenseEntry::ID_> Entry_;

		CategoryLink () = default;
		CategoryLink (const Category&, const NakedExpenseEntry&);

		static QString ClassName () { return "CategoryLink"; }

		static QString FieldNameMorpher (const QString& str) { return str; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Poleemery::CategoryLink,
		ID_,
		Category_,
		Entry_)

namespace LC
{
namespace Poleemery
{
	struct ReceiptEntry : EntryBase
	{
		using EntryBase::EntryBase;

		static QString ClassName () { return "ReceiptEntry"; }

		static QString FieldNameMorpher (const QString& str) { return str; }

		EntryType GetType () const { return EntryType::Receipt; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Poleemery::ReceiptEntry,
		ID_,
		AccountID_,
		Amount_,
		Name_,
		Description_,
		Date_)

namespace LC
{
namespace Poleemery
{
	struct Rate
	{
		Util::oral::PKey<int> ID_;

		QString Code_;
		QDateTime SnapshotTime_;
		double Rate_;

		static QString ClassName () { return "Rate"; }

		static QString FieldNameMorpher (const QString& str) { return str; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Poleemery::Rate,
		ID_,
		Code_,
		SnapshotTime_,
		Rate_)

namespace LC
{
namespace Poleemery
{
	struct BalanceInfo
	{
		double Total_;
		QHash<int, double> Accs_;
	};

	struct EntryWithBalance
	{
		EntryBase_ptr Entry_;
		BalanceInfo Balance_;
	};
}
}
