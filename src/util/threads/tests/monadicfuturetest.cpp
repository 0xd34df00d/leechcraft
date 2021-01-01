/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "monadicfuturetest.h"
#include <QtTest>
#include <monadicfuture.h>
#include "common.h"

QTEST_MAIN (LC::Util::MonadicFuturesTest)

namespace LC::Util
{
	void MonadicFuturesTest::testFunctor ()
	{
		QEventLoop loop;

		auto res = MkWaiter () (10) *
				[&loop] (int val)
				{
					QTimer::singleShot (0, &loop, SLOT (quit ()));
					return std::to_string (val);
				};

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (res.result (), std::string { "20" });
	}

	void MonadicFuturesTest::testFunctorReady ()
	{
		QEventLoop loop;

		auto res = Util::MakeReadyFuture<int> (10) *
				[&loop] (int val)
				{
					QTimer::singleShot (0, &loop, SLOT (quit ()));
					return std::to_string (val);
				};

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (res.result (), std::string { "10" });
	}
}
