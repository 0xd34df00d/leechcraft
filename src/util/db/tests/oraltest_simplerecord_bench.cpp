/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "oraltest_simplerecord_bench.h"
#include "common.h"
#include "simplerecord.h"

QTEST_GUILESS_MAIN (LeechCraft::Util::OralTest_SimpleRecord_Bench)

namespace LeechCraft
{
namespace Util
{
	namespace sph = oral::sph;

	void OralTest_SimpleRecord_Bench::benchSimpleRecordAdapt ()
	{
		if constexpr (OralBench)
		{
			auto db = MakeDatabase ();
			Util::oral::Adapt<SimpleRecord, OralFactory> (db);

			QBENCHMARK { Util::oral::Adapt<SimpleRecord> (db); }
		}
	}

	void OralTest_SimpleRecord_Bench::benchBaselineInsert ()
	{
		if constexpr (OralBench)
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
	}

	void OralTest_SimpleRecord_Bench::benchSimpleRecordInsert ()
	{
		if constexpr (OralBench)
		{
			auto db = MakeDatabase ();
			const auto& adapted = Util::oral::Adapt<SimpleRecord, OralFactory> (db);

			QBENCHMARK { adapted.Insert ({ 0, "0" }, lco::InsertAction::Ignore); }
		}
	}

	void OralTest_SimpleRecord_Bench::benchBaselineUpdate ()
	{
		if constexpr (OralBench)
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
	}

	void OralTest_SimpleRecord_Bench::benchSimpleRecordUpdate ()
	{
		if constexpr (OralBench)
		{
			auto db = MakeDatabase ();
			auto adapted = Util::oral::Adapt<SimpleRecord, OralFactory> (db);
			adapted.Insert ({ 0, "0" });

			QBENCHMARK { adapted.Update ({ 0, "1" }); }
		}
	}
}
}
