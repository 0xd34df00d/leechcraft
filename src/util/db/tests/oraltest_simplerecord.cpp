/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "oraltest_simplerecord.h"
#include "common.h"
#include "simplerecord.h"

QTEST_GUILESS_MAIN (LC::Util::OralTest_SimpleRecord)

namespace LC
{
namespace Util
{
	namespace sph = oral::sph;

	void OralTest_SimpleRecord::testSimpleRecordInsertSelect ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "0" }, { 1, "1" }, { 2, "2" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertReplaceSelect ()
	{
		auto db = MakeDatabase ();

		auto adapted = Util::oral::AdaptPtr<SimpleRecord, OralFactory> (db);
		for (int i = 0; i < 3; ++i)
			adapted->Insert (OralFactory {}, { 0, QString::number (i) }, lco::InsertAction::Replace::PKey);

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "2" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertIgnoreSelect ()
	{
		auto db = MakeDatabase ();

		auto adapted = Util::oral::AdaptPtr<SimpleRecord, OralFactory> (db);
		for (int i = 0; i < 3; ++i)
			adapted->Insert (OralFactory {}, { 0, QString::number (i) }, lco::InsertAction::Ignore);

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "0" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByPos ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::f<&SimpleRecord::ID_> == 1);
		QCOMPARE (list, (QList<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByPos2 ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::f<&SimpleRecord::ID_> < 2);
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "0" }, { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByPos3 ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::f<&SimpleRecord::ID_> < 2 && sph::f<&SimpleRecord::Value_> == QString { "1" });
		QCOMPARE (list, (QList<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectOneByPos ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& single = adapted->SelectOne (sph::f<&SimpleRecord::ID_> == 1);
		QCOMPARE (single, (std::optional<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByFields ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::f<&SimpleRecord::ID_> == 1);
		QCOMPARE (list, (QList<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByFields2 ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::f<&SimpleRecord::ID_> < 2);
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "0" }, { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByFields3 ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::f<&SimpleRecord::ID_> < 2 && sph::f<&SimpleRecord::Value_> == QString { "1" });
		QCOMPARE (list, (QList<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectOneByFields ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& single = adapted->SelectOne (sph::f<&SimpleRecord::ID_> == 1);
		QCOMPARE (single, (std::optional<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectSingleFieldByFields ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::fields<&SimpleRecord::Value_>, sph::f<&SimpleRecord::ID_> < 2);
		QCOMPARE (list, (QList<QString> { "0", "1" }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectFieldsByFields ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::fields<&SimpleRecord::ID_, &SimpleRecord::Value_>, sph::f<&SimpleRecord::ID_> < 2);
		QCOMPARE (list, (QList<std::tuple<int, QString>> { { 0, "0" }, { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectFieldsByFieldsOrderAsc ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::fields<&SimpleRecord::ID_, &SimpleRecord::Value_>,
				sph::f<&SimpleRecord::ID_> < 2,
				oral::OrderBy<sph::asc<&SimpleRecord::Value_>>);
		QCOMPARE (list, (QList<std::tuple<int, QString>> { { 0, "0" }, { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectFieldsByFieldsOrderDesc ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::fields<&SimpleRecord::ID_, &SimpleRecord::Value_>,
				sph::f<&SimpleRecord::ID_> < 2,
				oral::OrderBy<sph::desc<&SimpleRecord::Value_>>);
		QCOMPARE (list, (QList<std::tuple<int, QString>> { { 1, "1" }, { 0, "0" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectFieldsByFieldsOrderManyAsc ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::fields<&SimpleRecord::ID_, &SimpleRecord::Value_>,
				sph::f<&SimpleRecord::ID_> < 2,
				oral::OrderBy<sph::asc<&SimpleRecord::Value_>, sph::desc<&SimpleRecord::ID_>>);
		QCOMPARE (list, (QList<std::tuple<int, QString>> { { 0, "0" }, { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectFieldsByFieldsOrderManyDesc ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::fields<&SimpleRecord::ID_, &SimpleRecord::Value_>,
				sph::f<&SimpleRecord::ID_> < 2,
				oral::OrderBy<sph::desc<&SimpleRecord::Value_>, sph::asc<&SimpleRecord::ID_>>);
		QCOMPARE (list, (QList<std::tuple<int, QString>> { { 1, "1" }, { 0, "0" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectNoOffsetLimit ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase (), 10);
		const auto& list = adapted->Select.Build ().Limit (2) ();
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "0" }, { 1, "1" } }));
	}

	/*
	void OralTest_SimpleRecord::testSimpleRecordInsertSelectOffsetNoLimit ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase (), 10);
		const auto& list = adapted->Select.Build ().Offset ({ 8 }) ();
		QCOMPARE (list, (QList<SimpleRecord> { { 8, "8" }, { 9, "9" } }));
	}
	 */

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectOffsetLimit ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase (), 10);
		const auto& list = adapted->Select.Build ().Offset (5).Limit (2) ();
		QCOMPARE (list, (QList<SimpleRecord> { { 5, "5" }, { 6, "6" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectCount ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto count = adapted->Select (sph::count<>);
		QCOMPARE (count, 3);
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectCountByFields ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto count = adapted->Select (sph::count<>, sph::f<&SimpleRecord::ID_> < 2);
		QCOMPARE (count, 2);
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectMin ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto min = adapted->Select (sph::min<&SimpleRecord::ID_>);
		QCOMPARE (min, 0);
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectMax ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto max = adapted->Select (sph::max<&SimpleRecord::ID_>);
		QCOMPARE (max, 2);
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectMinPlusMax ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto minMax = adapted->Select (sph::min<&SimpleRecord::ID_> + sph::max<&SimpleRecord::ID_>);
		QCOMPARE (minMax, (std::tuple { 0, 2 }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectValuePlusMinPlusMax ()
	{
		auto adapted = oral::AdaptPtr<SimpleRecord, OralFactory> (MakeDatabase ());
		for (int i = 0; i < 3; ++i)
			adapted->Insert ({ i, "0" });
		for (int i = 3; i < 6; ++i)
			adapted->Insert ({ i, "1" });

		const auto allMinMax = adapted->Select.Build ()
				.Select (sph::fields<&SimpleRecord::Value_> + sph::min<&SimpleRecord::ID_> + sph::max<&SimpleRecord::ID_>)
				.Group (oral::GroupBy<&SimpleRecord::Value_>)
				();
		QCOMPARE (allMinMax, (QList<std::tuple<QString, int, int>> { { { "0" }, 0, 2 }, { { "1" }, 3, 5 } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectAllPlusMinPlusMax ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase (), 2);
		const auto allMinMax = adapted->Select.Build ()
				.Select (sph::all + sph::min<&SimpleRecord::ID_> + sph::max<&SimpleRecord::ID_>)
				.Group<&SimpleRecord::ID_, &SimpleRecord::Value_> ()
				();
		QCOMPARE (allMinMax, (QList<std::tuple<SimpleRecord, int, int>> { { { 0, "0" }, 0, 0 }, { { 1, "1" }, 1, 1 } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectLike ()
	{
		using namespace oral::infix;

		auto adapted = Util::oral::AdaptPtr<SimpleRecord, OralFactory> (MakeDatabase ());
		adapted->Insert ({ 0, "foo" });
		adapted->Insert ({ 1, "bar" });
		adapted->Insert ({ 2, "foobar" });
		const auto& list = adapted->Select (sph::f<&SimpleRecord::Value_> |like| QString { "%oo%" });
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "foo" }, { 2, "foobar" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordUpdate ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		adapted->Update ({ 0, "meh" });
		const auto updated = adapted->Select (sph::f<&SimpleRecord::ID_> == 0);
		QCOMPARE (updated, (QList<SimpleRecord> { { 0, "meh" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordUpdateExprTree ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		adapted->Update (sph::f<&SimpleRecord::Value_> = QString { "meh" }, sph::f<&SimpleRecord::ID_> == 0);
		const auto updated = adapted->Select (sph::f<&SimpleRecord::ID_> == 0);
		QCOMPARE (updated, (QList<SimpleRecord> { { 0, "meh" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordUpdateMultiExprTree ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		adapted->Update ((sph::f<&SimpleRecord::Value_> = QString { "meh" }, sph::f<&SimpleRecord::ID_> = 10),
				sph::f<&SimpleRecord::ID_> == 0);
		const auto updated = adapted->Select (sph::f<&SimpleRecord::ID_> == 10);
		QCOMPARE (updated, (QList<SimpleRecord> { { 10, "meh" } }));
	}
}
}
