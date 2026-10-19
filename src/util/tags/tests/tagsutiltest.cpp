/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsutiltest.h"
#include <QtTest>
#include <util/sll/qtutil.h>
#include "util.h"

QTEST_APPLESS_MAIN (LC::Util::TagsUtilTest)

namespace LC::Util
{
	void TagsUtilTest::testSplitTags_data ()
	{
		QTest::addColumn<QString> ("text");
		QTest::addColumn<QString> ("separator");
		QTest::addColumn<QStringList> ("expected");

		const auto sep = "; "_qs;
		const auto a = "a"_qs;
		const auto b = "b"_qs;

		QTest::newRow ("canonical") << "a; b"_qs << sep << QStringList { a, b };
		QTest::newRow ("single") << a << sep << QStringList { a };
		QTest::newRow ("empty") << QString {} << sep << QStringList {};
		QTest::newRow ("no space after separator") << "a;b"_qs << sep << QStringList { a, b };
		QTest::newRow ("whitespace around") << "  a ;  b  "_qs << sep << QStringList { a, b };
		QTest::newRow ("empty parts") << "a;; ;b;"_qs << sep << QStringList { a, b };
		QTest::newRow ("whitespace separator") << "a b"_qs << " "_qs << QStringList { a, b };
	}

	void TagsUtilTest::testSplitTags ()
	{
		QFETCH (QString, text);
		QFETCH (QString, separator);
		QFETCH (QStringList, expected);

		QCOMPARE (SplitTags (text, separator), expected);
	}
}
