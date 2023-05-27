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
#include <ctstringutils.h>

QTEST_APPLESS_MAIN (LC::Util::CtStringTest)

namespace LC::Util
{
#define TEST_STR "test string"
	void CtStringTest::testConstruction ()
	{
		constexpr CtString s { TEST_STR };
		QCOMPARE (s.ToString (), QString { TEST_STR });
	}

	void CtStringTest::testUnsizedConstruction ()
	{
		constexpr auto getStr = [] () constexpr { return TEST_STR; };

		constexpr auto s = CtString<StringBufSize (getStr ())>::FromUnsized (getStr ());
		QCOMPARE (s.ToString (), QString { TEST_STR });
	}

	void CtStringTest::testUDL ()
	{
		constexpr auto s = "test string"_ct;
		QCOMPARE (s.ToString (), QString { TEST_STR });
	}

	void CtStringTest::testConcat ()
	{
		constexpr auto s1 = "hello, "_ct;
		constexpr auto s2 = "world!"_ct;
		constexpr auto s3 = " how's life?"_ct;

		constexpr auto concat = s1 + s2 + s3;
		const QString expected { "hello, world! how's life?" };
		QCOMPARE (concat.ToString (), expected);

		constexpr auto concat2 = "hello, "_ct;
		QCOMPARE ((concat2 + "world!" + " how's life?").ToString (), expected);
	}

	void CtStringTest::testNub ()
	{
		constexpr static std::tuple input { "hello"_ct, "world"_ct, "hello"_ct, "lc"_ct, "what's"_ct, "up"_ct, "lc"_ct, "lc"_ct };
		constexpr std::tuple expected { "hello"_ct, "world"_ct, "lc"_ct, "what's"_ct, "up"_ct };

		constexpr static auto F = [&] { return input; };
		constexpr auto nubbed = Nub<F>();
		static_assert (nubbed == expected);
	}
}
