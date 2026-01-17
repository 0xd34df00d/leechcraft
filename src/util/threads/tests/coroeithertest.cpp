/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coroeithertest.h"
#include <QtTest>
#include <util/threads/coro.h>
#include <util/threads/coro/eithercoro.h>

QTEST_GUILESS_MAIN (LC::Util::CoroEitherTest)

namespace LC::Util
{
	using Ty = Either<char, int>;

	void CoroEitherTest::testSuccess ()
	{
		auto action = [] -> Ty
		{
			auto n = co_await Ty { 3 };
			auto m = co_await Ty { 5 };
			co_return n * m;
		};
		QCOMPARE (action (), 15);
	}

	void CoroEitherTest::testFailure ()
	{
		auto action = [] -> Ty
		{
			auto m = co_await Ty { 3 };
			auto n = co_await Ty { AsLeft, 'a' };
			auto k = co_await Ty { AsLeft, 'b' };
			co_return n * m * k;
		};
		QCOMPARE (action (), (Ty { AsLeft, 'a' }));
	}

	void CoroEitherTest::testException ()
	{
		auto action = [] -> Ty
		{
			auto m = co_await Ty { 3 };
			auto n = co_await [] -> Ty { throw std::runtime_error { "err" }; } ();
			auto k = co_await Ty { AsLeft, 'a' };
			co_return n * m * k;
		};
		QVERIFY_THROWS_EXCEPTION (std::runtime_error, action ());
	}
}
