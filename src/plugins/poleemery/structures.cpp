/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "structures.h"
#include <QtDebug>
#include <util/sll/unreachable.h>

namespace LC
{
namespace Poleemery
{
	QString ToHumanReadable (AccType type)
	{
		switch (type)
		{
		case AccType::BankAccount:
			return QObject::tr ("bank account");
		case AccType::Cash:
			return QObject::tr ("cash");
		}

		Util::Unreachable ();
	}

	bool operator== (const Account& a1, const Account& a2)
	{
		return a1.ID_ == a2.ID_ &&
				a1.Type_ == a2.Type_ &&
				a1.Name_ == a2.Name_ &&
				a1.Currency_ == a2.Currency_;
	}

	bool operator!= (const Account& a1, const Account& a2)
	{
		return !(a1 == a2);
	}

	EntryBase::EntryBase (int accId, double amount, const QString& name, const QString& descr, const QDateTime& dt)
	: AccountID_ { accId }
	, Amount_ { amount }
	, Name_ { name }
	, Description_ { descr }
	, Date_ { dt }
	{
	}

	NakedExpenseEntry::NakedExpenseEntry (int accId, double amount,
			const QString& name, const QString& descr, const QDateTime& dt,
			double count, const QString& shop, const QString& currency, double rate)
	: EntryBase { accId, amount, name, descr, dt, }
	, Count_ { count }
	, Shop_ { shop }
	, EntryCurrency_ { currency }
	, Rate_ { rate }
	{
	}

	ExpenseEntry::ExpenseEntry (const NakedExpenseEntry& naked)
	: NakedExpenseEntry (naked)
	{
	}

	ExpenseEntry::ExpenseEntry (int accId, double amount,
			const QString& name, const QString& descr, const QDateTime& dt,
			double count, const QString& shop, const QString& currency, double rate,
			const QStringList& cats)
	: NakedExpenseEntry { accId, amount, name, descr, dt, count, shop, currency, rate }
	, Categories_ { cats }
	{
	}

	Category::Category (const QString& name)
	: Name_ { name }
	{
	}

	CategoryLink::CategoryLink (const Category& category, const NakedExpenseEntry& entry)
	: Category_ { category.ID_ }
	, Entry_ { entry.ID_ }
	{
	}
}
}
