/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "oraltest.h"
#include "common.h"

QTEST_GUILESS_MAIN (LC::Util::OralTest)

using LC::operator""_ct;

struct AutogenPKeyRecord
{
	lco::PKey<int> ID_;
	QString Value_;

	constexpr static auto ClassName = "AutogenPKeyRecord"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Value_);
	}
};

ORAL_ADAPT_STRUCT (AutogenPKeyRecord,
		ID_,
		Value_)

TOSTRING (AutogenPKeyRecord)

struct NoPKeyRecord
{
	int ID_;
	QString Value_;

	constexpr static auto ClassName = "NoPKeyRecord"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Value_);
	}
};

ORAL_ADAPT_STRUCT (NoPKeyRecord,
		ID_,
		Value_)

TOSTRING (NoPKeyRecord)

struct NonInPlaceConstructibleRecord
{
	int ID_;
	QString Value_;

	NonInPlaceConstructibleRecord () = default;

	NonInPlaceConstructibleRecord (int id, const QString& value, double someExtraArgument)
	: ID_ { id }
	, Value_ { value }
	{
		Q_UNUSED (someExtraArgument)
	}

	constexpr static auto ClassName = "NonInPlaceConstructibleRecord"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Value_);
	}
};

ORAL_ADAPT_STRUCT (NonInPlaceConstructibleRecord,
		ID_,
		Value_)

TOSTRING (NonInPlaceConstructibleRecord)

struct ConstrainedAutogenPKeyRecord
{
	lco::PKey<int> ID_ {};
	lco::Unique<QString> City_;
	int Population_;

	constexpr static auto ClassName = "ConstrainedAutogenPKeyRecord"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, City_, Population_);
	}
};

ORAL_ADAPT_STRUCT (ConstrainedAutogenPKeyRecord,
		ID_,
		City_,
		Population_)

TOSTRING (ConstrainedAutogenPKeyRecord)

struct OptionalFieldRecord
{
	lco::PKey<int> ID_;
	QString Name_;
	std::optional<QString> NickName_;
	std::optional<QByteArray> Extra_;

	constexpr static auto ClassName = "OptionalFieldRecord"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Name_, NickName_, Extra_);
	}
};

ORAL_ADAPT_STRUCT (OptionalFieldRecord,
		ID_,
		Name_,
		NickName_,
		Extra_)

TOSTRING (OptionalFieldRecord)

struct ComplexConstraintsRecord
{
	int ID_;
	QString Name_;
	QString City_;
	int Age_;
	int Weight_;

	constexpr static auto ClassName = "ComplexConstraintsRecord"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Name_, City_, Age_, Weight_);
	}

	using Constraints = lco::Constraints<
				lco::PrimaryKey<0, 1>,
				lco::UniqueSubset<&ComplexConstraintsRecord::Age_, &ComplexConstraintsRecord::Weight_>
			>;
};

ORAL_ADAPT_STRUCT (ComplexConstraintsRecord,
		ID_,
		Name_,
		City_,
		Age_,
		Weight_)

TOSTRING (ComplexConstraintsRecord)

namespace LC
{
namespace Util
{
	namespace sph = oral::sph;

	void OralTest::testAutoPKeyRecordInsertSelect ()
	{
		qDebug () << oral::detail::FieldNames<AutogenPKeyRecord>;
		auto adapted = PrepareRecords<AutogenPKeyRecord> (MakeDatabase ());
		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<AutogenPKeyRecord> { { 1, "0" }, { 2, "1" }, { 3, "2" } }));
	}

	void OralTest::testAutoPKeyRecordInsertRvalueReturnsPKey ()
	{
		auto adapted = Util::oral::AdaptPtr<AutogenPKeyRecord, OralFactory> (MakeDatabase ());

		QList<int> ids;
		for (int i = 0; i < 3; ++i)
			ids << adapted->Insert ({ 0, QString::number (i) });

		QCOMPARE (ids, (QList<int> { 1, 2, 3 }));
	}

	void OralTest::testAutoPKeyRecordInsertConstLvalueReturnsPKey ()
	{
		auto adapted = Util::oral::AdaptPtr<AutogenPKeyRecord, OralFactory> (MakeDatabase ());

		QList<AutogenPKeyRecord> records;
		for (int i = 0; i < 3; ++i)
			records.push_back ({ 0, QString::number (i) });

		QList<int> ids;
		for (const auto& record : records)
			ids << adapted->Insert (record);

		QCOMPARE (ids, (QList<int> { 1, 2, 3 }));
	}

	void OralTest::testAutoPKeyRecordInsertSetsPKey ()
	{
		auto adapted = Util::oral::AdaptPtr<AutogenPKeyRecord, OralFactory> (MakeDatabase ());

		QList<AutogenPKeyRecord> records;
		for (int i = 0; i < 3; ++i)
			records.push_back ({ 0, QString::number (i) });

		for (auto& record : records)
			adapted->Insert (record);

		QCOMPARE (records, (QList<AutogenPKeyRecord> { { 1, "0" }, { 2, "1" }, { 3, "2" } }));
	}

	void OralTest::testNoPKeyRecordInsertSelect ()
	{
		auto adapted = PrepareRecords<NoPKeyRecord> (MakeDatabase ());
		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<NoPKeyRecord> { { 0, "0" }, { 1, "1" }, { 2, "2" } }));
	}

	void OralTest::testNonInPlaceConstructibleRecordInsertSelect ()
	{
		auto adapted = Util::oral::AdaptPtr<NonInPlaceConstructibleRecord, OralFactory> (MakeDatabase ());
		for (int i = 0; i < 3; ++i)
			adapted->Insert ({ i, QString::number (i), 0 });

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<NonInPlaceConstructibleRecord> { { 0, "0", 0 }, { 1, "1", 0 }, { 2, "2", 0 } }));
	}

	void OralTest::testComplexConstraintsRecordInsertSelectDefault ()
	{
		auto adapted = Util::oral::AdaptPtr<ComplexConstraintsRecord, OralFactory> (MakeDatabase ());

		adapted->Insert ({ 0, "first", "c1", 1, 2 });
		QVERIFY_THROWS_EXCEPTION (oral::QueryException, adapted->Insert ({ 0, "second", "c1", 1, 2 }));
		QVERIFY_THROWS_EXCEPTION (oral::QueryException, adapted->Insert ({ 0, "first", "c1", 1, 3 }));
		adapted->Insert ({ 0, "second", "c2", 1, 3 });
		QVERIFY_THROWS_EXCEPTION (oral::QueryException, adapted->Insert ({ 0, "first", "c1", 1, 3 }));

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<ComplexConstraintsRecord> { { 0, "first", "c1", 1, 2 }, { 0, "second", "c2", 1, 3 } }));
	}

	void OralTest::testComplexConstraintsRecordInsertSelectIgnore ()
	{
		auto adapted = Util::oral::AdaptPtr<ComplexConstraintsRecord, OralFactory> (MakeDatabase ());

		adapted->Insert ({ 0, "first", "c1", 1, 2 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "second", "c2", 1, 2 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "first", "c3", 1, 3 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "second", "c4", 1, 3 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "first", "c5", 1, 3 }, lco::InsertAction::Ignore);

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<ComplexConstraintsRecord> { { 0, "first", "c1", 1, 2 }, { 0, "second", "c4", 1, 3 } }));
	}

	void OralTest::testComplexConstraintsRecordInsertSelectReplace ()
	{
		auto adapted = Util::oral::AdaptPtr<ComplexConstraintsRecord, OralFactory> (MakeDatabase ());

		adapted->Insert ({ 0, "alice", "city1", 1, 2 });
		adapted->Insert ({ 0, "bob", "city2", 1, 2 },
				lco::InsertAction::Replace::Fields<&ComplexConstraintsRecord::Name_>);
		QCOMPARE (adapted->Select (), (QList<ComplexConstraintsRecord> { { 0, "bob", "city1", 1, 2 } }));

		adapted->Insert ({ 0, "alice", "city3", 2, 3 });
		QCOMPARE (adapted->Select (), (QList<ComplexConstraintsRecord> { { 0, "bob", "city1", 1, 2 }, { 0, "alice", "city3", 2, 3 } }));

		// cascading constraint violation: (0, "alice") ↦ (0, "bob") fails
		QVERIFY_THROWS_EXCEPTION (oral::QueryException,
				adapted->Insert ({ 1, "bob", "city4", 2, 3 },
						lco::InsertAction::Replace::Fields<&ComplexConstraintsRecord::Name_, &ComplexConstraintsRecord::City_>));

		adapted->Insert ({ 1, "bob", "city4", 2, 3 },
				lco::InsertAction::Replace::Fields<&ComplexConstraintsRecord::ID_, &ComplexConstraintsRecord::Name_, &ComplexConstraintsRecord::City_>);
		QCOMPARE (adapted->Select (), (QList<ComplexConstraintsRecord> { { 0, "bob", "city1", 1, 2 }, { 1, "bob", "city4", 2, 3 } }));
	}

	void OralTest::testConstrainedAutogenPKeyRecordInsertIgnore ()
	{
		using Rec = ConstrainedAutogenPKeyRecord;
		auto adapted = Util::oral::AdaptPtr<Rec, OralFactory> (MakeDatabase ());

		QCOMPARE (adapted->Insert ({ .City_ = "c1", .Population_ = 100 }), 1);
		QCOMPARE (adapted->Insert ({ .City_ = "c2", .Population_ = 200 }), 2);
		QCOMPARE (adapted->Select (), (QList<Rec> { { 1, "c1", 100 }, { 2, "c2", 200 } }));

		QCOMPARE (adapted->Insert ({ .City_ = "c1", .Population_ = 300 }, lco::InsertAction::Ignore), std::optional<int> {});

		QCOMPARE (adapted->Select (), (QList<Rec> { { 1, "c1", 100 }, { 2, "c2", 200 } }));
	}

	void OralTest::testConstrainedAutogenPKeyRecordInsertReplace ()
	{
		using Rec = ConstrainedAutogenPKeyRecord;
		auto adapted = Util::oral::AdaptPtr<Rec, OralFactory> (MakeDatabase ());

		QCOMPARE (adapted->Insert ({ .City_ = "c1", .Population_ = 100 }), 1);
		QCOMPARE (adapted->Insert ({ .City_ = "c2", .Population_ = 200 }), 2);
		QCOMPARE (adapted->Select (), (QList<Rec> { { 1, "c1", 100 }, { 2, "c2", 200 } }));

		QVERIFY_THROWS_EXCEPTION (oral::QueryException, adapted->Insert ({ .City_ = "c1", .Population_ = 300 }));

		QCOMPARE (adapted->Insert ({ .City_ = "c1", .Population_ = 300 }, lco::InsertAction::Replace::Whole), 1);
		QCOMPARE (adapted->Insert ({ .City_ = "c2", .Population_ = 400 }, lco::InsertAction::Replace::Whole), 2);
		QCOMPARE (adapted->Select (), (QList<Rec> { { 1, "c1", 300 }, { 2, "c2", 400 } }));
	}

	void OralTest::testOptionalFieldNullRoundTrip ()
	{
		using Rec = OptionalFieldRecord;
		auto adapted = Util::oral::AdaptPtr<Rec, OralFactory> (MakeDatabase ());

		adapted->Insert ({ {}, "alice", std::nullopt, std::nullopt });
		adapted->Insert ({ {}, "bob", std::nullopt, std::nullopt });

		const auto& list = adapted->Select ();
		QCOMPARE (list.size (), 2);
		QCOMPARE (list [0].NickName_, std::nullopt);
		QCOMPARE (list [0].Extra_, std::nullopt);
		QCOMPARE (list [1].NickName_, std::nullopt);
		QCOMPARE (list [1].Extra_, std::nullopt);
	}

	void OralTest::testOptionalFieldValueRoundTrip ()
	{
		using Rec = OptionalFieldRecord;
		auto adapted = Util::oral::AdaptPtr<Rec, OralFactory> (MakeDatabase ());

		adapted->Insert ({ {}, "alice", "Al", QByteArray { "data1" } });
		adapted->Insert ({ {}, "bob", std::nullopt, std::nullopt });

		const auto& list = adapted->Select ();
		QCOMPARE (list.size (), 2);
		QCOMPARE (list [0].NickName_, std::optional<QString> { "Al" });
		QCOMPARE (list [0].Extra_, std::optional<QByteArray> { "data1" });
		QCOMPARE (list [1].NickName_, std::nullopt);
		QCOMPARE (list [1].Extra_, std::nullopt);
	}

	void OralTest::testOptionalFieldUpdateNullToValue ()
	{
		using Rec = OptionalFieldRecord;
		auto adapted = Util::oral::AdaptPtr<Rec, OralFactory> (MakeDatabase ());

		adapted->Insert ({ {}, "alice", std::nullopt, std::nullopt });
		adapted->Update (sph::f<&Rec::NickName_> = QString { "Al" },
				sph::f<&Rec::Name_> == QString { "alice" });

		const auto& list = adapted->Select ();
		QCOMPARE (list.size (), 1);
		QCOMPARE (list [0].NickName_, std::optional<QString> { "Al" });
		QCOMPARE (list [0].Extra_, std::nullopt);
	}

	void OralTest::testOptionalFieldUpdateValueToNull ()
	{
		using Rec = OptionalFieldRecord;
		auto adapted = Util::oral::AdaptPtr<Rec, OralFactory> (MakeDatabase ());

		adapted->Insert ({ {}, "alice", "Al", QByteArray { "data" } });
		adapted->Update (sph::f<&Rec::NickName_> = std::optional<QString> {},
				sph::f<&Rec::Name_> == QString { "alice" });

		const auto& list = adapted->Select ();
		QCOMPARE (list.size (), 1);
		QCOMPARE (list [0].NickName_, std::nullopt);
		QCOMPARE (list [0].Extra_, std::optional<QByteArray> { "data" });
	}
}
}
