/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "corosignaltest.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <QThread>
#include <QTimer>
#include <QtTest>
#include <coro.h>
#include <coro/corocontext.h>
#include <coro/getresult.h>
#include <coro/signal.h>
#include <util/sll/qtutil.h>

QTEST_GUILESS_MAIN (LC::Util::CoroSignalTest)

namespace LC::Util
{
	void CoroSignalTest::testNoArgs ()
	{
		SignalEmitter emitter;
		bool resumed = false;
		auto task = [] (SignalEmitter& emitter, bool& resumed) -> Task<void>
		{
			co_await Signal { emitter, &SignalEmitter::NoArgs };
			resumed = true;
		} (emitter, resumed);

		QVERIFY (!resumed);
		emit emitter.NoArgs ();
		GetTaskResult (task);
		QVERIFY (resumed);
	}

	void CoroSignalTest::testOneArg ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<int>
		{
			co_return co_await Signal { emitter, &SignalEmitter::OneArg };
		} (emitter);

		emit emitter.OneArg (42);
		QCOMPARE (GetTaskResult (task), 42);
	}

	void CoroSignalTest::testTwoArgsYieldTuple ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<std::tuple<int, QString>>
		{
			co_return co_await Signal { emitter, &SignalEmitter::TwoArgs };
		} (emitter);

		emit emitter.TwoArgs (7, "seven"_qs);
		const auto& [number, name] = GetTaskResult (task);
		QCOMPARE (number, 7);
		QCOMPARE (name, "seven"_qs);
	}

	void CoroSignalTest::testRefArgYieldsOwnedValue ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<QString>
		{
			auto value = co_await Signal { emitter, &SignalEmitter::RefArg };
			static_assert (std::is_same_v<decltype (value), QString>);
			co_return value;
		} (emitter);

		emit emitter.RefArg (QString::number (42));
		QCOMPARE (GetTaskResult (task), "42"_qs);
	}

	void CoroSignalTest::testOverloaded ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<QString>
		{
			co_return co_await Signal { emitter, qOverload<const QString&> (&SignalEmitter::Overloaded) };
		} (emitter);

		emit emitter.Overloaded (1);
		QVERIFY (emitter.IsConnected (qOverload<const QString&> (&SignalEmitter::Overloaded)));
		emit emitter.Overloaded ("string"_qs);
		QCOMPARE (GetTaskResult (task), "string"_qs);
	}

	void CoroSignalTest::testPrivateTagStripped ()
	{
		QTimer timer;
		timer.setSingleShot (true);
		auto timedOut = [] (QTimer& timer) -> Task<void>
		{
			co_return co_await Signal { timer, &QTimer::timeout };
		} (timer);
		timer.start (0);
		GetTaskResult (timedOut);

		QObject object;
		auto renamed = [] (QObject& object) -> Task<QString>
		{
			auto name = co_await Signal { object, &QObject::objectNameChanged };
			static_assert (std::is_same_v<decltype (name), QString>);
			co_return name;
		} (object);
		object.setObjectName ("renamed"_qs);
		QCOMPARE (GetTaskResult (renamed), "renamed"_qs);
	}

	void CoroSignalTest::testBaseClassThroughDerived ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		auto task = [] (SignalEmitter& emitter) -> Task<QObject*>
		{
			co_return co_await Signal { emitter, &QObject::destroyed };
		} (*emitter);

		QObject *expected = emitter.get ();
		emitter.reset ();
		QCOMPARE (GetTaskResult (task), expected);
	}

	void CoroSignalTest::testResumesAfterEmitReturns ()
	{
		SignalEmitter emitter;
		int stage = 0;
		int stageAtResume = -1;
		auto task = [] (SignalEmitter& emitter, const int& stage, int& stageAtResume) -> Task<void>
		{
			co_await Signal { emitter, &SignalEmitter::NoArgs };
			stageAtResume = stage;
		} (emitter, stage, stageAtResume);

		emit emitter.NoArgs ();
		stage = 1;
		GetTaskResult (task);
		QCOMPARE (stageAtResume, 1);
	}

	void CoroSignalTest::testContinuationMayDestroySender ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		auto task = [] (std::unique_ptr<SignalEmitter>& emitter) -> Task<void>
		{
			co_await Signal { *emitter, &SignalEmitter::NoArgs };
			emitter.reset ();
		} (emitter);

		emit emitter->NoArgs ();
		QVERIFY (emitter);
		GetTaskResult (task);
		QVERIFY (!emitter);
	}

	void CoroSignalTest::testFirstEmissionWins ()
	{
		SignalEmitter emitter;
		int resumes = 0;
		auto task = [] (SignalEmitter& emitter, int& resumes) -> Task<int>
		{
			const auto value = co_await Signal { emitter, &SignalEmitter::OneArg };
			++resumes;
			co_return value;
		} (emitter, resumes);

		emit emitter.OneArg (1);
		emit emitter.OneArg (2);
		QCOMPARE (GetTaskResult (task), 1);

		emit emitter.OneArg (3);
		QTest::qWait (10);
		QCOMPARE (resumes, 1);
	}

	void CoroSignalTest::testConnectedOnlyWhileSuspended ()
	{
		SignalEmitter emitter;
		QVERIFY (!emitter.IsConnected (&SignalEmitter::OneArg));

		auto task = [] (SignalEmitter& emitter) -> Task<int>
		{
			co_return co_await Signal { emitter, &SignalEmitter::OneArg };
		} (emitter);
		QVERIFY (emitter.IsConnected (&SignalEmitter::OneArg));

		emit emitter.OneArg (5);
		QCOMPARE (GetTaskResult (task), 5);
		QVERIFY (!emitter.IsConnected (&SignalEmitter::OneArg));
	}

	void CoroSignalTest::testManyAwaitersOneEmission ()
	{
		SignalEmitter emitter;
		const auto awaitOneArg = [] (SignalEmitter& emitter) -> Task<int>
		{
			co_return co_await Signal { emitter, &SignalEmitter::OneArg };
		};
		auto first = awaitOneArg (emitter);
		auto second = awaitOneArg (emitter);

		emit emitter.OneArg (9);
		QCOMPARE (GetTaskResult (first), 9);
		QCOMPARE (GetTaskResult (second), 9);
	}

	void CoroSignalTest::testReawaitSame ()
	{
		SignalEmitter emitter;
		int counter = 0;
		QTimer ticker;
		ticker.callOnTimeout ([&] { emit emitter.OneArg (++counter); });
		ticker.start (0);

		auto task = [] (SignalEmitter& emitter) -> Task<QList<int>>
		{
			const Signal signal { emitter, &SignalEmitter::OneArg };
			QList<int> values;
			while (values.size () < 3)
				values << co_await signal;
			co_return values;
		} (emitter);

		const auto values = GetTaskResult (task);
		ticker.stop ();
		QCOMPARE (values.size (), 3);
		QVERIFY (std::ranges::adjacent_find (values, std::greater_equal {}) == values.end ());
	}

	void CoroSignalTest::testContextDeathDisconnects ()
	{
		SignalEmitter emitter;
		auto context = std::make_unique<CoroContext> ();
		auto task = [] (CoroContext *context, SignalEmitter& emitter) -> ContextTask<int>
		{
			co_await AddContext { *context };
			co_return co_await Signal { emitter, &SignalEmitter::OneArg };
		} (&*context, emitter);
		QVERIFY (emitter.IsConnected (&SignalEmitter::OneArg));

		context.reset ();
		QVERIFY (!emitter.IsConnected (&SignalEmitter::OneArg));

		emit emitter.OneArg (42);
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (task));
	}

	void CoroSignalTest::testEmittedFromAnotherThread ()
	{
		QThread worker;
		worker.start ();

		SignalEmitter emitter;
		emitter.moveToThread (&worker);

		QThread *resumedOn = nullptr;
		auto task = [] (SignalEmitter& emitter, QThread*& resumedOn) -> Task<int>
		{
			const auto value = co_await Signal { emitter, &SignalEmitter::OneArg };
			resumedOn = QThread::currentThread ();
			co_return value;
		} (emitter, resumedOn);

		const auto mainThread = QThread::currentThread ();
		QThread *emittedOn = nullptr;
		QMetaObject::invokeMethod (&emitter,
				[&]
				{
					emittedOn = QThread::currentThread ();
					emit emitter.OneArg (11);
					emitter.moveToThread (mainThread);
				},
				Qt::QueuedConnection);

		QCOMPARE (GetTaskResult (task), 11);
		QCOMPARE (resumedOn, mainThread);

		worker.quit ();
		QVERIFY (worker.wait ());
		QCOMPARE (emittedOn, &worker);
	}
}
