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

#include "oraltest_simplerecord.h"
#include "common.h"
#include "simplerecord.h"

QTEST_GUILESS_MAIN (LeechCraft::Util::OralTest_SimpleRecord)

namespace LeechCraft
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
			adapted->Insert ({ 0, QString::number (i) }, lco::InsertAction::Replace::PKey<SimpleRecord>);

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "2" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertIgnoreSelect ()
	{
		auto db = MakeDatabase ();

		auto adapted = Util::oral::AdaptPtr<SimpleRecord, OralFactory> (db);
		for (int i = 0; i < 3; ++i)
			adapted->Insert ({ 0, QString::number (i) }, lco::InsertAction::Ignore);

		const auto& list = adapted->Select ();
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "0" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByPos ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::_0 == 1);
		QCOMPARE (list, (QList<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByPos2 ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::_0 < 2);
		QCOMPARE (list, (QList<SimpleRecord> { { 0, "0" }, { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectByPos3 ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& list = adapted->Select (sph::_0 < 2 && sph::_1 == QString { "1" });
		QCOMPARE (list, (QList<SimpleRecord> { { 1, "1" } }));
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectOneByPos ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto& single = adapted->SelectOne (sph::_0 == 1);
		QCOMPARE (single, (boost::optional<SimpleRecord> { { 1, "1" } }));
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
		QCOMPARE (single, (boost::optional<SimpleRecord> { { 1, "1" } }));
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

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectCount ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto count = adapted->Select (sph::count);
		QCOMPARE (count, 3);
	}

	void OralTest_SimpleRecord::testSimpleRecordInsertSelectCountByFields ()
	{
		auto adapted = PrepareRecords<SimpleRecord> (MakeDatabase ());
		const auto count = adapted->Select (sph::count, sph::f<&SimpleRecord::ID_> < 2);
		QCOMPARE (count, 2);
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
