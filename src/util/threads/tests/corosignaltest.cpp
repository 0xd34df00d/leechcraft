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
#include <optional>
#include <stdexcept>
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
		auto task = [] (SignalEmitter& emitter) -> Task<bool>
		{
			co_return co_await Signal { emitter, &SignalEmitter::NoArgs };
		} (emitter);

		emit emitter.NoArgs ();
		QVERIFY (GetTaskResult (task));
	}

	void CoroSignalTest::testOneArg ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<int>
		{
			auto value = co_await Signal { emitter, &SignalEmitter::OneArg };
			static_assert (std::is_same_v<decltype (value), std::optional<int>>);
			co_return value.value ();
		} (emitter);

		emit emitter.OneArg (42);
		QCOMPARE (GetTaskResult (task), 42);
	}

	void CoroSignalTest::testTwoArgsYieldTuple ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<std::tuple<int, QString>>
		{
			auto value = co_await Signal { emitter, &SignalEmitter::TwoArgs };
			static_assert (std::is_same_v<decltype (value), std::optional<std::tuple<int, QString>>>);
			co_return value.value ();
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
			static_assert (std::is_same_v<decltype (value), std::optional<QString>>);
			co_return value.value ();
		} (emitter);

		emit emitter.RefArg (QString::number (42));
		QCOMPARE (GetTaskResult (task), "42"_qs);
	}

	void CoroSignalTest::testOverloaded ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<QString>
		{
			co_return (co_await Signal { emitter, qOverload<const QString&> (&SignalEmitter::Overloaded) }).value ();
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
		auto timedOut = [] (QTimer& timer) -> Task<bool>
		{
			co_return co_await Signal { timer, &QTimer::timeout };
		} (timer);
		timer.start (0);
		QVERIFY (GetTaskResult (timedOut));

		QObject object;
		auto renamed = [] (QObject& object) -> Task<QString>
		{
			auto name = co_await Signal { object, &QObject::objectNameChanged };
			static_assert (std::is_same_v<decltype (name), std::optional<QString>>);
			co_return name.value ();
		} (object);
		object.setObjectName ("renamed"_qs);
		QCOMPARE (GetTaskResult (renamed), "renamed"_qs);
	}

	void CoroSignalTest::testBaseClassThroughDerived ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		auto task = [] (SignalEmitter& emitter) -> Task<std::optional<QObject*>>
		{
			co_return co_await Signal { emitter, &QObject::destroyed };
		} (*emitter);

		QObject *expected = emitter.get ();
		emitter.reset ();
		const auto result = GetTaskResult (task);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, expected);
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
			const auto value = (co_await Signal { emitter, &SignalEmitter::OneArg }).value ();
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
		QVERIFY (!emitter.IsConnected (&QObject::destroyed));

		auto task = [] (SignalEmitter& emitter) -> Task<int>
		{
			co_return (co_await Signal { emitter, &SignalEmitter::OneArg }).value ();
		} (emitter);
		QVERIFY (emitter.IsConnected (&SignalEmitter::OneArg));
		QVERIFY (emitter.IsConnected (&QObject::destroyed));

		emit emitter.OneArg (5);
		QCOMPARE (GetTaskResult (task), 5);
		QVERIFY (!emitter.IsConnected (&SignalEmitter::OneArg));
		QVERIFY (!emitter.IsConnected (&QObject::destroyed));
	}

	void CoroSignalTest::testManyAwaitersOneEmission ()
	{
		SignalEmitter emitter;
		const auto awaitOneArg = [] (SignalEmitter& emitter) -> Task<int>
		{
			co_return (co_await Signal { emitter, &SignalEmitter::OneArg }).value ();
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
				values << (co_await signal).value ();
			co_return values;
		} (emitter);

		const auto values = GetTaskResult (task);
		ticker.stop ();
		QCOMPARE (values.size (), 3);
		QVERIFY (std::ranges::adjacent_find (values, std::greater_equal {}) == values.end ());
	}

	void CoroSignalTest::testHandlerResultIsAwaited ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<double>
		{
			auto value = co_await Signal { emitter, &SignalEmitter::OneArg, [] (int value) { return value / 2.0; } };
			static_assert (std::is_same_v<decltype (value), std::optional<double>>);
			co_return value.value ();
		} (emitter);

		emit emitter.OneArg (21);
		QCOMPARE (GetTaskResult (task), 10.5);
	}

	void CoroSignalTest::testHandlerRunsInsideEmit ()
	{
		SignalEmitter emitter;
		int stage = 0;
		int stageInHandler = -1;
		int stageAtResume = -1;
		auto task = [] (SignalEmitter& emitter, const int& stage, int& stageInHandler, int& stageAtResume) -> Task<void>
		{
			co_await Signal { emitter, &SignalEmitter::NoArgs, [&] { stageInHandler = stage; return true; } };
			stageAtResume = stage;
		} (emitter, stage, stageInHandler, stageAtResume);

		emit emitter.NoArgs ();
		stage = 1;
		GetTaskResult (task);
		QCOMPARE (stageInHandler, 0);
		QCOMPARE (stageAtResume, 1);
	}

	void CoroSignalTest::testHandlerSnapshotsDyingSender ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		emitter->setObjectName ("snapshot"_qs);
		auto task = [] (SignalEmitter& emitter) -> Task<std::optional<QString>>
		{
			co_return co_await Signal { emitter, &SignalEmitter::NoArgs, [&emitter] { return emitter.objectName (); } };
		} (*emitter);

		emit emitter->NoArgs ();
		emitter.reset ();
		const auto result = GetTaskResult (task);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, "snapshot"_qs);
	}

	void CoroSignalTest::testHandlerTupleArgs ()
	{
		SignalEmitter emitter;
		auto task = [] (SignalEmitter& emitter) -> Task<QString>
		{
			const auto handler = [] (int number, const QString& name) { return name + u':' + QString::number (number); };
			co_return (co_await Signal { emitter, &SignalEmitter::TwoArgs, handler }).value ();
		} (emitter);

		emit emitter.TwoArgs (7, "seven"_qs);
		QCOMPARE (GetTaskResult (task), "seven:7"_qs);
	}

	void CoroSignalTest::testHandlerPrivateTagStripped ()
	{
		QTimer timer;
		timer.setSingleShot (true);
		auto timedOut = [] (QTimer& timer) -> Task<std::optional<int>>
		{
			co_return co_await Signal { timer, &QTimer::timeout, [] { return 42; } };
		} (timer);
		timer.start (0);
		QCOMPARE (GetTaskResult (timedOut).value (), 42);

		QObject object;
		auto renamed = [] (QObject& object) -> Task<QString>
		{
			co_return (co_await Signal { object, &QObject::objectNameChanged, [] (const QString& name) { return name.toUpper (); } }).value ();
		} (object);
		object.setObjectName ("renamed"_qs);
		QCOMPARE (GetTaskResult (renamed), "RENAMED"_qs);
	}

	void CoroSignalTest::testHandlerRunsOnce ()
	{
		SignalEmitter emitter;
		int calls = 0;
		auto task = [] (SignalEmitter& emitter, int& calls) -> Task<int>
		{
			co_return (co_await Signal { emitter, &SignalEmitter::OneArg, [&calls] (int value) { ++calls; return value; } }).value ();
		} (emitter, calls);

		emit emitter.OneArg (1);
		emit emitter.OneArg (2);
		QCOMPARE (GetTaskResult (task), 1);
		QCOMPARE (calls, 1);
	}

	void CoroSignalTest::testHandlerSenderDeathYieldsNullopt ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		bool handlerRan = false;
		auto task = [] (SignalEmitter& emitter, bool& handlerRan) -> Task<std::optional<int>>
		{
			co_return co_await Signal { emitter, &SignalEmitter::OneArg, [&handlerRan] (int value) { handlerRan = true; return value; } };
		} (*emitter, handlerRan);

		emitter.reset ();
		QVERIFY (!GetTaskResult (task).has_value ());
		QVERIFY (!handlerRan);
	}

	void CoroSignalTest::testHandlerExceptionRethrownAtAwait ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		int resumes = 0;
		QString caught;
		auto task = [] (SignalEmitter& emitter, int& resumes, QString& caught) -> Task<void>
		{
			try
			{
				co_await Signal { emitter, &SignalEmitter::OneArg, [] (int) -> int { throw std::runtime_error { "boom" }; } };
			}
			catch (const std::runtime_error& e)
			{
				caught = QString::fromUtf8 (e.what ());
			}
			++resumes;
		} (*emitter, resumes, caught);

		emit emitter->OneArg (1);
		emitter.reset ();
		GetTaskResult (task);
		QCOMPARE (caught, "boom"_qs);
		QTest::qWait (10);
		QCOMPARE (resumes, 1);
	}

	void CoroSignalTest::testHandlerCopiedPerAwait ()
	{
		SignalEmitter emitter;
		QTimer ticker;
		ticker.callOnTimeout ([&] { emit emitter.OneArg (10); });
		ticker.start (0);

		auto task = [] (SignalEmitter& emitter) -> Task<QList<int>>
		{
			const Signal signal { emitter, &SignalEmitter::OneArg, [calls = 0] (int value) mutable { return value + calls++; } };
			QList<int> values;
			while (values.size () < 3)
				values << (co_await signal).value ();
			co_return values;
		} (emitter);

		const auto values = GetTaskResult (task);
		ticker.stop ();
		QCOMPARE (values, (QList<int> { 10, 10, 10 }));
	}

	void CoroSignalTest::testSenderDeathYieldsNullopt ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		bool resumed = false;
		auto noArgs = [] (SignalEmitter& emitter) -> Task<bool>
		{
			co_return co_await Signal { emitter, &SignalEmitter::NoArgs };
		} (*emitter);
		auto oneArg = [] (SignalEmitter& emitter, bool& resumed) -> Task<std::optional<int>>
		{
			const auto value = co_await Signal { emitter, &SignalEmitter::OneArg };
			resumed = true;
			co_return value;
		} (*emitter, resumed);
		auto twoArgs = [] (SignalEmitter& emitter) -> Task<std::optional<std::tuple<int, QString>>>
		{
			co_return co_await Signal { emitter, &SignalEmitter::TwoArgs };
		} (*emitter);

		emitter.reset ();
		QVERIFY (!resumed);

		QVERIFY (!GetTaskResult (noArgs));
		QVERIFY (!GetTaskResult (oneArg).has_value ());
		QVERIFY (!GetTaskResult (twoArgs).has_value ());
	}

	void CoroSignalTest::testEmissionBeforeDeathKeepsValue ()
	{
		auto emitter = std::make_unique<SignalEmitter> ();
		int resumes = 0;
		auto task = [] (SignalEmitter& emitter, int& resumes) -> Task<std::optional<int>>
		{
			const auto value = co_await Signal { emitter, &SignalEmitter::OneArg };
			++resumes;
			co_return value;
		} (*emitter, resumes);

		emit emitter->OneArg (5);
		emitter.reset ();

		const auto result = GetTaskResult (task);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, 5);
		QTest::qWait (10);
		QCOMPARE (resumes, 1);
	}

	void CoroSignalTest::testContextDeathDisconnects ()
	{
		SignalEmitter emitter;
		auto context = std::make_unique<CoroContext> ();
		auto task = [] (CoroContext *context, SignalEmitter& emitter) -> ContextTask<int>
		{
			co_await AddContext { *context };
			co_return (co_await Signal { emitter, &SignalEmitter::OneArg }).value ();
		} (&*context, emitter);
		QVERIFY (emitter.IsConnected (&SignalEmitter::OneArg));

		context.reset ();
		QVERIFY (!emitter.IsConnected (&SignalEmitter::OneArg));
		QVERIFY (!emitter.IsConnected (&QObject::destroyed));

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
			const auto value = (co_await Signal { emitter, &SignalEmitter::OneArg }).value ();
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
