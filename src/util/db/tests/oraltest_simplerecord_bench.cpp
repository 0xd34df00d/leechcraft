/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "oraltest_simplerecord_bench.h"
#include "common.h"
#include "simplerecord.h"

QTEST_GUILESS_MAIN (LC::Util::OralTest_SimpleRecord_Bench)

namespace LC
{
namespace Util
{
	namespace sph = oral::sph;

	void OralTest_SimpleRecord_Bench::benchSimpleRecordAdapt ()
	{
		auto db = MakeDatabase ();
		Util::oral::Adapt<SimpleRecord, OralFactory> (db);

		QBENCHMARK { Util::oral::Adapt<SimpleRecord> (db); }
	}

	void OralTest_SimpleRecord_Bench::benchBaselineInsert ()
	{
		auto db = MakeDatabase ();
		Util::oral::Adapt<SimpleRecord, OralFactory> (db);

		QSqlQuery query { db };
		query.prepare ("INSERT OR IGNORE INTO SimpleRecord (ID, Value) VALUES (:id, :val);");

		QBENCHMARK
		{
			query.bindValue (":id", 0);
			query.bindValue (":val", "0");
			query.exec ();
		}
	}

	void OralTest_SimpleRecord_Bench::benchSimpleRecordInsert ()
	{
		auto db = MakeDatabase ();
		const auto& adapted = Util::oral::Adapt<SimpleRecord, OralFactory> (db);

		QBENCHMARK { adapted.Insert ({ 0, "0" }, lco::InsertAction::Ignore); }
	}

	void OralTest_SimpleRecord_Bench::benchBaselineUpdate ()
	{
		auto db = MakeDatabase ();
		const auto& adapted = Util::oral::Adapt<SimpleRecord, OralFactory> (db);
		adapted.Insert ({ 0, "0" });

		QSqlQuery query { db };
		query.prepare ("UPDATE SimpleRecord SET Value = :val WHERE Id = :id;");

		QBENCHMARK
		{
			query.bindValue (":id", 0);
			query.bindValue (":val", "1");
			query.exec ();
		}
	}

	void OralTest_SimpleRecord_Bench::benchSimpleRecordUpdate ()
	{
		auto db = MakeDatabase ();
		auto adapted = Util::oral::Adapt<SimpleRecord, OralFactory> (db);
		adapted.Insert ({ 0, "0" });

		QBENCHMARK { adapted.Update ({ 0, "1" }); }
	}
}
}
