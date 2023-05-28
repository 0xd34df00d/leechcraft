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

struct AutogenPKeyRecord
{
	lco::PKey<int> ID_;
	QString Value_;

	constexpr static auto ClassName ()
	{
		using namespace LC;
		return "AutogenPKeyRecord"_ct;
	}

	auto AsTuple () const
	{
		return std::tie (ID_, Value_);
	}
};

BOOST_FUSION_ADAPT_STRUCT (AutogenPKeyRecord,
		ID_,
		Value_)

TOSTRING (AutogenPKeyRecord)

struct NoPKeyRecord
{
	int ID_;
	QString Value_;

	constexpr static auto ClassName ()
	{
		using namespace LC;
		return "NoPKeyRecord"_ct;
	}

	auto AsTuple () const
	{
		return std::tie (ID_, Value_);
	}
};

BOOST_FUSION_ADAPT_STRUCT (NoPKeyRecord,
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

	constexpr static auto ClassName ()
	{
		using namespace LC;
		return "NonInPlaceConstructibleRecord"_ct;
	}

	auto AsTuple () const
	{
		return std::tie (ID_, Value_);
	}
};

BOOST_FUSION_ADAPT_STRUCT (NonInPlaceConstructibleRecord,
		ID_,
		Value_)

TOSTRING (NonInPlaceConstructibleRecord)

struct ComplexConstraintsRecord
{
	int ID_;
	QString Value_;
	int Age_;
	int Weight_;

	constexpr static auto ClassName ()
	{
		using namespace LC;
		return "ComplexConstraintsRecord"_ct;
	}

	auto AsTuple () const
	{
		return std::tie (ID_, Value_, Age_, Weight_);
	}

	using Constraints = lco::Constraints<
				lco::PrimaryKey<0, 1>,
				lco::UniqueSubset<2, 3>
			>;
};

BOOST_FUSION_ADAPT_STRUCT (ComplexConstraintsRecord,
		ID_,
		Value_,
		Age_,
		Weight_)

TOSTRING (ComplexConstraintsRecord)

template<typename... Args>
QDebug operator<< (QDebug dbg, const std::tuple<Args...>& tup)
{
	return std::apply ([&] (auto&&... args) { return ((dbg.nospace () << args << ' '), ...); }, tup);
}

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

	namespace
	{
		template<typename Ex, typename F>
		void ShallThrow (F&& f)
		{
			bool failed = false;
			try
			{
				f ();
			}
			catch (const Ex&)
			{
				failed = true;
			}

			QCOMPARE (failed, true);
		}
	}

	void OralTest::testComplexConstraintsRecordInsertSelectDefault ()
	{
		auto adapted = Util::oral::AdaptPtr<ComplexConstraintsRecord, OralFactory> (MakeDatabase ());

		adapted->Insert ({ 0, "first", 1, 2 });
		ShallThrow<oral::QueryException> ([&] { adapted->Insert ({ 0, "second", 1, 2 }); });
		ShallThrow<oral::QueryException> ([&] { adapted->Insert ({ 0, "first", 1, 3 }); });
		adapted->Insert ({ 0, "second", 1, 3 });
		ShallThrow<oral::QueryException> ([&] { adapted->Insert ({ 0, "first", 1, 3 }); });

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<ComplexConstraintsRecord> { { 0, "first", 1, 2 }, { 0, "second", 1, 3 } }));
	}

	void OralTest::testComplexConstraintsRecordInsertSelectIgnore ()
	{
		auto adapted = Util::oral::AdaptPtr<ComplexConstraintsRecord, OralFactory> (MakeDatabase ());

		adapted->Insert ({ 0, "first", 1, 2 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "second", 1, 2 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "first", 1, 3 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "second", 1, 3 }, lco::InsertAction::Ignore);
		adapted->Insert ({ 0, "first", 1, 3 }, lco::InsertAction::Ignore);

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<ComplexConstraintsRecord> { { 0, "first", 1, 2 }, { 0, "second", 1, 3 } }));
	}

	void OralTest::testComplexConstraintsRecordInsertSelectReplace ()
	{
		auto adapted = Util::oral::AdaptPtr<ComplexConstraintsRecord, OralFactory> (MakeDatabase ());

		const auto idValueFields = lco::InsertAction::Replace::Fields<
				&ComplexConstraintsRecord::ID_,
				&ComplexConstraintsRecord::Value_
			>;
		const auto weightAgeFields = lco::InsertAction::Replace::Fields<
				&ComplexConstraintsRecord::Weight_,
				&ComplexConstraintsRecord::Age_
			>;
		adapted->Insert ({ 0, "first", 1, 2 }, idValueFields);
		adapted->Insert ({ 0, "second", 1, 2 }, weightAgeFields);
		adapted->Insert ({ 0, "first", 1, 3 }, idValueFields);
		adapted->Insert ({ 0, "third", 1, 3 }, weightAgeFields);
		adapted->Insert ({ 0, "first", 1, 3 }, weightAgeFields);

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<ComplexConstraintsRecord> { {0, "second", 1, 2 }, { 0, "first", 1, 3 } }));
	}
}
}
