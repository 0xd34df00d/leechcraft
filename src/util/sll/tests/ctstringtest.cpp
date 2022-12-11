/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ctstringtest.h"
#include <QtTest>
#include <ctstring.h>

QTEST_APPLESS_MAIN (LC::Util::CtStringTest)

namespace LC::Util
{
#define TEST_STR "test string"
	void CtStringTest::testConstruction ()
	{
		constexpr CtString s { TEST_STR };
		QCOMPARE (s.GetRawSized (), TEST_STR);
	}

	void CtStringTest::testUnsizedConstruction ()
	{
		constexpr auto getStr = [] () constexpr { return TEST_STR; };

		constexpr auto s = CtString<StringBufSize (getStr ())>::FromUnsized (getStr ());
		QCOMPARE (s.GetRawSized (), TEST_STR);
	}

	void CtStringTest::testUDL ()
	{
		constexpr auto s = "test string"_ct;
		QCOMPARE (s.GetRawSized (), TEST_STR);
	}

	void CtStringTest::testConcat ()
	{
		constexpr auto s1 = "hello, "_ct;
		constexpr auto s2 = "world!"_ct;
		constexpr auto s3 = " how's life?"_ct;

		constexpr auto concat = s1 + s2 + s3;
		QCOMPARE (concat.GetRawSized (), "hello, world! how's life?");
	}
}
