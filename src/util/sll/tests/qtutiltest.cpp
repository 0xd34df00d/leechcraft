/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "qtutiltest.h"
#include <QtTest>
#include <qtutil.h>

QTEST_APPLESS_MAIN (LC::Util::QtUtilTest)

namespace LC::Util
{
	void QtUtilTest::testStringUDL ()
	{
		const auto& foo1 = u"foo"_qs;
		const auto& bar = u"bar"_qs;
		QCOMPARE (foo1, "foo");
		QCOMPARE (bar, "bar");

		auto foo2 = u"foo"_qs;
		QCOMPARE (foo2, "foo");

		foo2.chop (1);
		QCOMPARE (foo2, "fo");
		QCOMPARE (foo1, "foo");
	}
}

