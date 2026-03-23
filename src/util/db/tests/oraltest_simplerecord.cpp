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

using LC::operator""_ct;

// Models the pagination scenario: messages with a timestamp (TS) and an
// auto-generated primary key (Id) that don't necessarily correlate.
// After history sync, a message from the past gets a high Id.
struct MessageLikeRecord
{
	lco::PKey<int> Id_;
	int TS_;
	QString Body_;

	constexpr static auto ClassName = "MessageLikeRecord"_ct;

	auto AsTuple () const
	{
		return std::tuple (Id_, TS_, Body_);
	}
};

ORAL_ADAPT_STRUCT (MessageLikeRecord,
		Id_,
		TS_,
		Body_)

TOSTRING (MessageLikeRecord)

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
			adapted->Insert (OralFactory {}, { 0, QString::number (i) }, lco::InsertAction::Replace::Whole);

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

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectBuilderAndWhere ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select.Build ()
				.Where (sph::f<&SimpleRecord::ID_> < 2)
				.AndWhere (sph::f<&SimpleRecord::Value_> == QString { "1" })
				();
		QCOMPARE (list, (QList<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectBuilderAndWhereMultiple ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase (), 10);
		const auto& list = adapted->Select.Build ()
				.Where (sph::f<&SimpleRecord::ID_> < 8)
				.AndWhere (sph::f<&SimpleRecord::ID_> >= 2)
				.AndWhere (sph::f<&SimpleRecord::Value_> == QString { "5" })
				();
		QCOMPARE (list, (QList<SimpleRecord> { { 5, "5" } }));
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

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectMinPlusMaxEmpty ()
	{
		auto adapted = oral::AdaptPtr<SimpleRecord, OralFactory> (MakeDatabase ());
		const auto minMax = adapted->Select (sph::min<&SimpleRecord::ID_> + sph::max<&SimpleRecord::ID_>);
		QCOMPARE (minMax, (std::tuple { std::nullopt, std::nullopt }));
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
		QCOMPARE (allMinMax, (QList<std::tuple<QString, std::optional<int>, std::optional<int>>>
				{
					{ { "0" }, 0, 2 },
					{ { "1" }, 3, 5 }
				}));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectAllPlusMinPlusMax ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase (), 2);
		const auto allMinMax = adapted->Select.Build ()
				.Select (sph::all + sph::min<&SimpleRecord::ID_> + sph::max<&SimpleRecord::ID_>)
				.Group<&SimpleRecord::ID_, &SimpleRecord::Value_> ()
				();
		QCOMPARE (allMinMax, (QList<std::tuple<SimpleRecord, std::optional<int>, std::optional<int>>>
				{
					{ { 0, "0" }, 0, 0 }, { { 1, "1" }, 1, 1 }
				}));
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

	auto PrepareMessageLikeRecords (QSqlDatabase db)
	{
		using Rec = MessageLikeRecord;
		auto adapted = oral::AdaptPtr<Rec, OralFactory> (db);
		adapted->Insert ({ {}, 300, "c" });
		adapted->Insert ({ {}, 400, "d" });
		adapted->Insert ({ {}, 500, "e" });
		adapted->Insert ({ {}, 100, "a" });
		adapted->Insert ({ {}, 200, "b" });
		return adapted;
	}

	void OralTest_SimpleRecord::testTupleCompareEq ()
	{
		using Rec = MessageLikeRecord;
		auto adapted = PrepareMessageLikeRecords (MakeDatabase ());
		const auto& list = adapted->Select (sph::tuple<&Rec::TS_, &Rec::Id_> == std::tuple (300, 1));
		QCOMPARE (list, (QList<Rec> { { 1, 300, "c" } }));
	}

	void OralTest_SimpleRecord::testTupleCompareLt ()
	{
		using Rec = MessageLikeRecord;
		auto adapted = PrepareMessageLikeRecords (MakeDatabase ());
		const auto& list = adapted->Select.Build ()
				.Where (sph::tuple<&Rec::TS_, &Rec::Id_> < std::tuple (300, 1))
				.Order (oral::OrderBy<sph::asc<&Rec::TS_>, sph::asc<&Rec::Id_>>)
				();
		QCOMPARE (list, (QList<Rec> { { 4, 100, "a" }, { 5, 200, "b" } }));
	}

	void OralTest_SimpleRecord::testTupleCompareGt ()
	{
		using Rec = MessageLikeRecord;
		auto adapted = PrepareMessageLikeRecords (MakeDatabase ());
		const auto& list = adapted->Select.Build ()
				.Where (sph::tuple<&Rec::TS_, &Rec::Id_> > std::tuple (300, 1))
				.Order (oral::OrderBy<sph::asc<&Rec::TS_>, sph::asc<&Rec::Id_>>)
				();
		QCOMPARE (list, (QList<Rec> { { 2, 400, "d" }, { 3, 500, "e" } }));
	}

	void OralTest_SimpleRecord::testTupleCompareLte ()
	{
		using Rec = MessageLikeRecord;
		auto adapted = PrepareMessageLikeRecords (MakeDatabase ());
		const auto& list = adapted->Select.Build ()
				.Where (sph::tuple<&Rec::TS_, &Rec::Id_> <= std::tuple (300, 1))
				.Order (oral::OrderBy<sph::asc<&Rec::TS_>, sph::asc<&Rec::Id_>>)
				();
		QCOMPARE (list, (QList<Rec> { { 4, 100, "a" }, { 5, 200, "b" }, { 1, 300, "c" } }));
	}

	void OralTest_SimpleRecord::testTupleCompareGte ()
	{
		using Rec = MessageLikeRecord;
		auto adapted = PrepareMessageLikeRecords (MakeDatabase ());
		const auto& list = adapted->Select.Build ()
				.Where (sph::tuple<&Rec::TS_, &Rec::Id_> >= std::tuple (300, 1))
				.Order (oral::OrderBy<sph::asc<&Rec::TS_>, sph::asc<&Rec::Id_>>)
				();
		QCOMPARE (list, (QList<Rec> { { 1, 300, "c" }, { 2, 400, "d" }, { 3, 500, "e" } }));
	}

	void OralTest_SimpleRecord::testTupleCompareIsNotComponentwise ()
	{
		using Rec = MessageLikeRecord;
		auto adapted = PrepareMessageLikeRecords (MakeDatabase ());
		const auto& before = adapted->Select (sph::tuple<&Rec::TS_, &Rec::Id_> < std::tuple (200, 5));
		QCOMPARE (before, (QList<Rec> { { 4, 100, "a" } }));

		const auto& after = adapted->Select.Build ()
				.Where (sph::tuple<&Rec::TS_, &Rec::Id_> > std::tuple (200, 5))
				.Order (oral::OrderBy<sph::asc<&Rec::TS_>, sph::asc<&Rec::Id_>>)
				();
		QCOMPARE (after, (QList<Rec> { { 1, 300, "c" }, { 2, 400, "d" }, { 3, 500, "e" } }));
	}

	void OralTest_SimpleRecord::testTupleCompareInBuilder ()
	{
		using Rec = MessageLikeRecord;
		auto adapted = PrepareMessageLikeRecords (MakeDatabase ());
		auto page = adapted->Select.Build ()
				.Where (sph::tuple<&Rec::TS_, &Rec::Id_> < std::tuple (400, 2))
				.Order (oral::OrderBy<sph::desc<&Rec::TS_>, sph::desc<&Rec::Id_>>)
				.Limit (2)
				();
		QCOMPARE (page, (QList<Rec> { { 1, 300, "c" }, { 5, 200, "b" } }));
	}
}
}
