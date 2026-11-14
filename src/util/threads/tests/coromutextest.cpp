/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coromutextest.h"
#include <algorithm>
#include <chrono>
#include <memory>
#include <vector>
#include <QTest>
#include <util/sll/void.h>
#include "coro.h"
#include "coro/channel.h"
#include "coro/coromutex.h"
#include "coro/getresult.h"

QTEST_GUILESS_MAIN (LC::Util::CoroMutexTest)

namespace LC::Util
{
	namespace
	{
		using Gate = Channel<Void>;

		void Open (Gate& gate)
		{
			gate.Send (Void {});
		}

		ContextTask<void> Section (CoroContext& context, CoroMutex& mutex, Gate& gate, QStringList& log, QString name)
		{
			co_await AddContext { context };
			{
				const auto guard = co_await mutex.Lock ();
				log << name + " in";
				co_await gate;
				log << name + " out";
			}
			log << name + (mutex.IsLocked () ? " handed over" : " unlocked");
		}

		Task<void> CountedSection (CoroMutex& mutex, int& inside, int& maxInside, int& finished)
		{
			using namespace std::chrono_literals;

			const auto guard = co_await mutex.Lock ();
			maxInside = std::max (maxInside, ++inside);
			co_await 1ms;
			--inside;
			++finished;
		}
	}

	void CoroMutexTest::testFreeMutexIsLockedRightAway ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		QVERIFY (!mutex.IsLocked ());

		auto section = Section (context, mutex, gate, log, "a");
		QCOMPARE (log, (QStringList { "a in" }));
		QVERIFY (mutex.IsLocked ());

		Open (gate);
		QTRY_COMPARE (log, (QStringList { "a in", "a out", "a unlocked" }));
		QVERIFY (!mutex.IsLocked ());
	}

	void CoroMutexTest::testSectionsDontOverlap ()
	{
		constexpr auto sectionsCount = 16;

		CoroMutex mutex;
		int inside = 0;
		int maxInside = 0;
		int finished = 0;
		std::vector<Task<void>> sections;
		for (int i = 0; i < sectionsCount; ++i)
			sections.push_back (CountedSection (mutex, inside, maxInside, finished));

		QTRY_COMPARE (finished, sectionsCount);
		QCOMPARE (maxInside, 1);
		QVERIFY (!mutex.IsLocked ());
	}

	void CoroMutexTest::testUnlockDefersWakeup ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto first = Section (context, mutex, gate, log, "a");
		auto second = Section (context, mutex, gate, log, "b");
		QCOMPARE (log, (QStringList { "a in" }));

		Open (gate);
		Open (gate);
		QTRY_COMPARE (log, (QStringList { "a in", "a out", "a handed over", "b in", "b out", "b unlocked" }));
	}

	void CoroMutexTest::testCancelledWaiterDoesntGetMutex ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto doomedContext = std::make_unique<CoroContext> ();
		auto holder = Section (context, mutex, gate, log, "holder");
		auto doomed = Section (*doomedContext, mutex, gate, log, "doomed");
		auto survivor = Section (context, mutex, gate, log, "survivor");
		doomedContext.reset ();

		Open (gate);
		Open (gate);
		QTRY_COMPARE (log, (QStringList
				{
					"holder in", "holder out", "holder handed over",
					"survivor in", "survivor out", "survivor unlocked"
				}));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (doomed));
	}

	void CoroMutexTest::testCancelledHolderUnlocks ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto doomedContext = std::make_unique<CoroContext> ();
		auto doomed = Section (*doomedContext, mutex, gate, log, "doomed");
		auto survivor = Section (context, mutex, gate, log, "survivor");
		doomedContext.reset ();

		Open (gate);
		QTRY_COMPARE (log, (QStringList { "doomed in", "survivor in", "survivor out", "survivor unlocked" }));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (doomed));
	}

	void CoroMutexTest::testCancelledWaiterHandsMutexBack ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto holderContext = std::make_unique<CoroContext> ();
		auto waiterContext = std::make_unique<CoroContext> ();
		auto holder = Section (*holderContext, mutex, gate, log, "holder");
		auto waiter = Section (*waiterContext, mutex, gate, log, "waiter");

		// the mutex is now on its way to the waiter, which is yet to be resumed
		holderContext.reset ();
		QVERIFY (mutex.IsLocked ());

		waiterContext.reset ();
		QVERIFY (!mutex.IsLocked ());

		auto survivor = Section (context, mutex, gate, log, "survivor");
		Open (gate);
		QTRY_COMPARE (log, (QStringList { "holder in", "survivor in", "survivor out", "survivor unlocked" }));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (holder));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (waiter));
	}

	namespace
	{
		struct Owner : QObject
		{
			CoroMutex Mutex_;

			ContextTask<void> RunSection (Gate& gate, QStringList& log, QString name)
			{
				return Section (CoroContext_, Mutex_, gate, log, name);
			}

			LC_CORO_CONTEXT
		};
	}

	void CoroMutexTest::testOwnerDiesWithSectionsInFlight ()
	{
		Gate gate;
		QStringList log;
		auto owner = std::make_unique<Owner> ();
		auto holder = owner->RunSection (gate, log, "holder");
		auto waiter = owner->RunSection (gate, log, "waiter");
		owner.reset ();

		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (holder));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (waiter));
		QCOMPARE (log, (QStringList { "holder in" }));
	}
}
