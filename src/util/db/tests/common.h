/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <tuple>
#include <QtTest>
#include <QSqlError>
#include <oral/oral.h>

namespace lco = LC::Util::oral;

using OralFactory = lco::SQLiteImplFactory;

template<typename T, typename = decltype (T {}.AsTuple ())>
auto operator== (const T& left, const T& right)
{
	return left.AsTuple () == right.AsTuple ();
}

namespace LC::Util::oral
{
	template<typename T, typename... Args>
	char* toString (const PKey<T, Args...>& pkey)
	{
		return QTest::toString (pkey.Val_);
	}
}

#define TOSTRING(n) inline char* toString (const n& rec) { return toString (#n, rec); }

template<typename T, typename TupleType = decltype (T {}.AsTuple ())>
char* toString (const char *name, const T& t)
{
	using QTest::toString;

	QByteArray ba { name };
	ba.append (" { ");

	std::apply ([&ba] (const auto&... args) { (ba.append (toString (args)).append (", "), ...); }, t.AsTuple ());

	if (std::tuple_size<TupleType>::value >= 1)
		ba.chop (2);
	ba.append (" }");

	return qstrdup (ba.data ());
}

namespace LC::Util
{
	QSqlDatabase MakeDatabase (const QString& name = ":memory:")
	{
		auto db = QSqlDatabase::addDatabase ("QSQLITE", Util::GenConnectionName ("TestConnection"));
		db.setDatabaseName (name);
		if (!db.open ())
			throw std::runtime_error { "cannot create test database" };

		RunTextQuery (db, "PRAGMA foreign_keys = ON;");

		return db;
	}

	template<typename T>
	auto PrepareRecords (QSqlDatabase db, int count = 3)
	{
		auto adapted = Util::oral::AdaptPtr<T, OralFactory> (db);
		for (int i = 0; i < count; ++i)
			adapted->Insert ({ i, QString::number (i) });
		return adapted;
	}
}
