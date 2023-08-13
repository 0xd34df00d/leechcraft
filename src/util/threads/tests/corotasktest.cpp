/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "corotasktest.h"
#include <QEventLoop>
#include <QtTest>
#include <coro.h>

QTEST_GUILESS_MAIN (LC::Util::CoroTaskTest)

namespace LC::Util
{
	namespace
	{
		template<typename T>
		T GetTaskResult (Task<T> task)
		{
			T result;

			QEventLoop loop;
			bool done = false;
			[&] () -> Task<void>
			{
				result = co_await task;
				done = true;
				loop.quit ();
			} ();
			if (!done)
				loop.exec ();

			return result;
		}
	}

	void CoroTaskTest::testReturn ()
	{
		auto task = [] () -> Task<int> { co_return 42; } ();
		auto result = GetTaskResult (task);
		QCOMPARE (result, 42);
	}

	void CoroTaskTest::testWait ()
	{
		QElapsedTimer timer;
		timer.start ();

		auto task = [] () -> Task<int>
		{
			using namespace std::chrono_literals;
			co_await 100ms;
			co_await Precisely { 10ms };
			co_return 42;
		} ();

		auto result = GetTaskResult (task);
		QCOMPARE (result, 42);
		QCOMPARE (timer.elapsed () > 100, true);
	}
}
