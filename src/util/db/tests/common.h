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

#define ORAL_FACTORY_SQLITE 1
#define ORAL_FACTORY_POSTGRES 2

#if ORAL_FACTORY == ORAL_FACTORY_SQLITE

using OralFactory = lco::SQLiteImplFactory;

#elif ORAL_FACTORY == ORAL_FACTORY_POSTGRES

#include <oral/pgimpl.h>

using OralFactory = lco::PostgreSQLImplFactory;

#else

#error "Unknown oral tests factory"

#endif

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
#if ORAL_FACTORY == ORAL_FACTORY_SQLITE
		auto db = QSqlDatabase::addDatabase ("QSQLITE", Util::GenConnectionName ("TestConnection"));
		db.setDatabaseName (name);
		if (!db.open ())
			throw std::runtime_error { "cannot create test database" };

		RunTextQuery (db, "PRAGMA foreign_keys = ON;");

		return db;
#elif ORAL_FACTORY == ORAL_FACTORY_POSTGRES
		Q_UNUSED (name)

		auto db = QSqlDatabase::addDatabase ("QPSQL", Util::GenConnectionName ("TestConnection"));

		db.setHostName ("localhost");
		db.setPort (5432);
		db.setUserName (qgetenv ("TEST_POSTGRES_USERNAME"));

		if (!db.open ())
		{
			DBLock::DumpError (db.lastError ());
			throw std::runtime_error { "cannot create test database" };
		}

		return db;
#endif
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
