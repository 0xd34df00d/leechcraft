/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagschangetest.h"
#include <QtTest>
#include <util/sll/qtutil.h>
#include "tagschange.h"

QTEST_APPLESS_MAIN (LC::Util::TagsChangeTest)

namespace LC::Util
{
	namespace
	{
		const auto A = "a"_qs;
		const auto B = "b"_qs;
		const auto C = "c"_qs;
		const auto D = "d"_qs;
	}

	void TagsChangeTest::testDifference_data ()
	{
		QTest::addColumn<QStringList> ("before");
		QTest::addColumn<QStringList> ("after");
		QTest::addColumn<QStringList> ("added");
		QTest::addColumn<QStringList> ("removed");

		QTest::newRow ("removal") << QStringList { A, B } << QStringList { A } << QStringList {} << QStringList { B };
		QTest::newRow ("addition") << QStringList { A } << QStringList { A, D } << QStringList { D } << QStringList {};
		QTest::newRow ("both") << QStringList { A, B } << QStringList { B, C } << QStringList { C } << QStringList { A };
		QTest::newRow ("none") << QStringList {} << QStringList {} << QStringList {} << QStringList {};
		QTest::newRow ("same") << QStringList { A, B } << QStringList { A, B } << QStringList {} << QStringList {};
		QTest::newRow ("reordering") << QStringList { A, B } << QStringList { B, A } << QStringList {} << QStringList {};
		QTest::newRow ("repetitions") << QStringList { A, A } << QStringList { D, A, D } << QStringList { D } << QStringList {};
		QTest::newRow ("order of after") << QStringList {} << QStringList { D, C } << QStringList { D, C } << QStringList {};
		QTest::newRow ("order of before") << QStringList { D, C } << QStringList {} << QStringList {} << QStringList { D, C };
	}

	void TagsChangeTest::testDifference ()
	{
		QFETCH (QStringList, before);
		QFETCH (QStringList, after);
		QFETCH (QStringList, added);
		QFETCH (QStringList, removed);

		const TagsChange change { .Before_ = before, .After_ = after };
		QCOMPARE (change.Added (), added);
		QCOMPARE (change.Removed (), removed);
		QCOMPARE (change.IsEmpty (), added.isEmpty () && removed.isEmpty ());
	}

	void TagsChangeTest::testApplyTo_data ()
	{
		QTest::addColumn<QStringList> ("tags");
		QTest::addColumn<QStringList> ("before");
		QTest::addColumn<QStringList> ("after");
		QTest::addColumn<QStringList> ("expected");

		QTest::newRow ("removal") << QStringList { A, B, C } << QStringList { A, B } << QStringList { A } << QStringList { A, C };
		QTest::newRow ("addition") << QStringList { C } << QStringList { A } << QStringList { A, D } << QStringList { C, D };
		QTest::newRow ("only the difference") << QStringList { C } << QStringList { A, B } << QStringList { A, B, D } << QStringList { C, D };
		QTest::newRow ("no change") << QStringList { C, D } << QStringList { A, B } << QStringList { A, B } << QStringList { C, D };
		QTest::newRow ("reordering") << QStringList { B, A } << QStringList { A, B } << QStringList { B, A } << QStringList { B, A };
		QTest::newRow ("added already present") << QStringList { D, C } << QStringList {} << QStringList { D } << QStringList { D, C };
		QTest::newRow ("added twice") << QStringList {} << QStringList {} << QStringList { D, D } << QStringList { D };
		QTest::newRow ("removed twice present") << QStringList { A, C, A } << QStringList { A } << QStringList {} << QStringList { C };
	}

	void TagsChangeTest::testApplyTo ()
	{
		QFETCH (QStringList, tags);
		QFETCH (QStringList, before);
		QFETCH (QStringList, after);
		QFETCH (QStringList, expected);

		QCOMPARE ((TagsChange { .Before_ = before, .After_ = after }.ApplyTo (tags)), expected);
	}
}
