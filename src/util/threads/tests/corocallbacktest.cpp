/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "corocallbacktest.h"
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <QTest>
#include <coro.h>
#include <coro/callback.h>
#include <coro/corocontext.h>
#include <coro/getresult.h>
#include <util/sll/qtutil.h>

QTEST_GUILESS_MAIN (LC::Util::CoroCallbackTest)

namespace LC::Util
{
	void CoroCallbackTest::testNoArgs ()
	{
		std::function<void ()> callback;
		auto task = [] (std::function<void ()>& callback) -> Task<bool>
		{
			Callback<> cb;
			callback = cb.GetCallback ();
			auto invoked = co_await cb;
			static_assert (std::is_same_v<decltype (invoked), bool>);
			co_return invoked;
		} (callback);

		callback ();
		QVERIFY (GetTaskResult (task));
	}

	void CoroCallbackTest::testOneArg ()
	{
		std::function<void (int)> callback;
		auto task = [] (std::function<void (int)>& callback) -> Task<int>
		{
			Callback<int> cb;
			callback = cb.GetCallback ();
			auto value = co_await cb;
			static_assert (std::is_same_v<decltype (value), std::optional<int>>);
			co_return value.value ();
		} (callback);

		callback (42);
		QCOMPARE (GetTaskResult (task), 42);
	}

	void CoroCallbackTest::testTwoArgsYieldTuple ()
	{
		std::function<void (int, QString)> callback;
		auto task = [] (std::function<void (int, QString)>& callback) -> Task<std::tuple<int, QString>>
		{
			Callback<int, QString> cb;
			callback = cb.GetCallback ();
			auto value = co_await cb;
			static_assert (std::is_same_v<decltype (value), std::optional<std::tuple<int, QString>>>);
			co_return value.value ();
		} (callback);

		callback (7, "seven"_qs);
		const auto& [number, name] = GetTaskResult (task);
		QCOMPARE (number, 7);
		QCOMPARE (name, "seven"_qs);
	}

	void CoroCallbackTest::testRefParameterYieldsOwnedValue ()
	{
		std::function<void (const QString&)> callback;
		auto task = [] (std::function<void (const QString&)>& callback) -> Task<QString>
		{
			Callback<QString> cb;
			callback = cb.GetCallback ();
			co_return (co_await cb).value ();
		} (callback);

		callback (QString::number (42));
		QCOMPARE (GetTaskResult (task), "42"_qs);
	}

	void CoroCallbackTest::testMoveOnlyArgs ()
	{
		using Ptr = std::unique_ptr<int>;

		std::function<void (Ptr)> oneArgCallback;
		auto oneArg = [] (std::function<void (Ptr)>& callback) -> Task<Ptr>
		{
			Callback<Ptr> cb;
			callback = cb.GetCallback ();
			auto value = co_await cb;
			static_assert (std::is_same_v<decltype (value), std::optional<Ptr>>);
			co_return std::move (value).value ();
		} (oneArgCallback);

		std::function<void (int, Ptr)> twoArgsCallback;
		auto twoArgs = [] (std::function<void (int, Ptr)>& callback) -> Task<std::tuple<int, Ptr>>
		{
			Callback<int, Ptr> cb;
			callback = cb.GetCallback ();
			auto value = co_await cb;
			static_assert (std::is_same_v<decltype (value), std::optional<std::tuple<int, Ptr>>>);
			co_return std::move (value).value ();
		} (twoArgsCallback);

		oneArgCallback (std::make_unique<int> (42));
		const auto pointer = GetTaskResult (oneArg);
		QVERIFY (pointer);
		QCOMPARE (*pointer, 42);

		twoArgsCallback (7, std::make_unique<int> (42));
		const auto [number, tuplePointer] = GetTaskResult (twoArgs);
		QCOMPARE (number, 7);
		QVERIFY (tuplePointer);
		QCOMPARE (*tuplePointer, 42);
	}

	void CoroCallbackTest::testAwaitedFromContextTask ()
	{
		std::function<void (int)> callback;
		CoroContext context;
		auto task = [] (CoroContext& context, std::function<void (int)>& callback) -> ContextTask<int>
		{
			co_await AddContext { context };

			Callback<int> cb;
			callback = cb.GetCallback ();
			co_return (co_await cb).value ();
		} (context, callback);

		callback (42);
		QCOMPARE (GetTaskResult (task), 42);
	}

	void CoroCallbackTest::testInvokedBeforeAwait ()
	{
		std::optional<int> result;
		bool finished = false;
		[] (std::optional<int>& result, bool& finished) -> Task<void>
		{
			Callback<int> cb;
			const std::function<void (int)> callee { cb.GetCallback () };
			callee (42);
			result = co_await cb;
			finished = true;
		} (result, finished);

		QVERIFY (finished);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, 42);
	}

	void CoroCallbackTest::testResumesAfterCallbackReturns ()
	{
		std::function<void ()> callback;
		int stage = 0;
		int stageAtResume = -1;
		auto task = [] (std::function<void ()>& callback, const int& stage, int& stageAtResume) -> Task<void>
		{
			Callback<> cb;
			callback = cb.GetCallback ();
			co_await cb;
			stageAtResume = stage;
		} (callback, stage, stageAtResume);

		callback ();
		stage = 1;
		GetTaskResult (task);
		QCOMPARE (stageAtResume, 1);
	}

	void CoroCallbackTest::testContinuationMayDestroyCallee ()
	{
		auto callback = std::make_unique<std::function<void ()>> ();
		auto task = [] (std::unique_ptr<std::function<void ()>>& callback) -> Task<void>
		{
			Callback<> cb;
			*callback = cb.GetCallback ();
			co_await cb;
			callback.reset ();
		} (callback);

		(*callback) ();
		QVERIFY (callback);
		GetTaskResult (task);
		QVERIFY (!callback);
	}

	void CoroCallbackTest::testSecondInvocationIsIgnored ()
	{
		std::function<void (int)> callback;
		int resumes = 0;
		auto task = [] (std::function<void (int)>& callback, int& resumes) -> Task<int>
		{
			Callback<int> cb;
			callback = cb.GetCallback ();
			const auto value = (co_await cb).value ();
			++resumes;
			co_return value;
		} (callback, resumes);

		callback (1);
		callback (2);
		QCOMPARE (GetTaskResult (task), 1);

		callback (3);
		QTest::qWait (10);
		QCOMPARE (resumes, 1);
	}

	void CoroCallbackTest::testInvocationAfterCompletion ()
	{
		std::function<void (int)> callback;
		auto task = [] (std::function<void (int)>& callback) -> Task<int>
		{
			Callback<int> cb;
			callback = cb.GetCallback ();
			co_return (co_await cb).value ();
		} (callback);

		// never invoked before the coroutine is done, so it has not let go of the Callback on its own
		auto lateCopy = callback;

		callback (1);
		QCOMPARE (GetTaskResult (task), 1);

		lateCopy (2);
		lateCopy = nullptr;
		callback = nullptr;
	}

	void CoroCallbackTest::testSecondGetCallbackThrows ()
	{
		Callback<int> cb;
		const std::function<void (int)> callback { cb.GetCallback () };
		QVERIFY_THROWS_EXCEPTION (std::runtime_error, cb.GetCallback ());

		auto task = [] (Callback<int>& cb) -> Task<std::optional<int>>
		{
			co_return co_await cb;
		} (cb);

		callback (42);
		const auto result = GetTaskResult (task);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, 42);
	}

	void CoroCallbackTest::testCopiesAreNotDrops ()
	{
		std::function<void (int)> callback;
		auto task = [] (std::function<void (int)>& callback) -> Task<std::optional<int>>
		{
			Callback<int> cb;
			callback = cb.GetCallback ();
			co_return co_await cb;
		} (callback);

		{
			const auto copy = callback;
			const std::function<void (int)> another { copy };
		}
		QTest::qWait (10);

		callback (42);
		const auto result = GetTaskResult (task);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, 42);
	}

	void CoroCallbackTest::testDroppedYieldsNullopt ()
	{
		std::function<void ()> noArgsCallback;
		std::function<void (int)> oneArgCallback;
		std::function<void (int, QString)> twoArgsCallback;
		bool resumed = false;

		auto noArgs = [] (std::function<void ()>& callback) -> Task<bool>
		{
			Callback<> cb;
			callback = cb.GetCallback ();
			co_return co_await cb;
		} (noArgsCallback);
		auto oneArg = [] (std::function<void (int)>& callback, bool& resumed) -> Task<std::optional<int>>
		{
			Callback<int> cb;
			callback = cb.GetCallback ();
			const auto value = co_await cb;
			resumed = true;
			co_return value;
		} (oneArgCallback, resumed);
		auto twoArgs = [] (std::function<void (int, QString)>& callback) -> Task<std::optional<std::tuple<int, QString>>>
		{
			Callback<int, QString> cb;
			callback = cb.GetCallback ();
			co_return co_await cb;
		} (twoArgsCallback);

		noArgsCallback = nullptr;
		oneArgCallback = nullptr;
		twoArgsCallback = nullptr;
		QVERIFY (!resumed);

		QVERIFY (!GetTaskResult (noArgs));
		QVERIFY (!GetTaskResult (oneArg).has_value ());
		QVERIFY (!GetTaskResult (twoArgs).has_value ());
	}

	void CoroCallbackTest::testDroppedBeforeAwait ()
	{
		std::optional<int> result { 0 };
		bool finished = false;
		[] (std::optional<int>& result, bool& finished) -> Task<void>
		{
			Callback<int> cb;
			{
				const std::function<void (int)> discarded { cb.GetCallback () };
			}
			result = co_await cb;
			finished = true;
		} (result, finished);

		QVERIFY (finished);
		QVERIFY (!result.has_value ());
	}

	void CoroCallbackTest::testInvocationBeforeDropKeepsValue ()
	{
		std::function<void (int)> callback;
		int resumes = 0;
		auto task = [] (std::function<void (int)>& callback, int& resumes) -> Task<std::optional<int>>
		{
			Callback<int> cb;
			callback = cb.GetCallback ();
			const auto value = co_await cb;
			++resumes;
			co_return value;
		} (callback, resumes);

		callback (5);
		callback = nullptr;

		const auto result = GetTaskResult (task);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, 5);
		QTest::qWait (10);
		QCOMPARE (resumes, 1);
	}

	void CoroCallbackTest::testContextDeathDetaches ()
	{
		std::function<void (int)> callback;
		auto context = std::make_unique<CoroContext> ();
		int resumes = 0;
		auto task = [] (CoroContext *context, std::function<void (int)>& callback, int& resumes) -> ContextTask<int>
		{
			co_await AddContext { *context };

			Callback<int> cb;
			callback = cb.GetCallback ();
			const auto value = (co_await cb).value ();
			++resumes;
			co_return value;
		} (&*context, callback, resumes);

		context.reset ();

		callback (42);
		callback = nullptr;
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (task));
		QTest::qWait (10);
		QCOMPARE (resumes, 0);
	}

	void CoroCallbackTest::testContextDeathAfterInvocation ()
	{
		std::function<void (int)> callback;
		auto context = std::make_unique<CoroContext> ();
		int resumes = 0;
		auto task = [] (CoroContext *context, std::function<void (int)>& callback, int& resumes) -> ContextTask<int>
		{
			co_await AddContext { *context };

			Callback<int> cb;
			callback = cb.GetCallback ();
			const auto value = (co_await cb).value ();
			++resumes;
			co_return value;
		} (&*context, callback, resumes);

		callback (42);
		context.reset ();

		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (task));
		QTest::qWait (10);
		QCOMPARE (resumes, 0);
	}

	namespace
	{
		ContextTask<std::optional<int>> Doomed (CoroContext& context, Callback<int>& cb, int& resumes)
		{
			co_await AddContext { context };
			const auto value = co_await cb;
			++resumes;
			co_return value;
		}

		enum class Completion
		{
			InvokedBeforeDeath,
			InvokedAfterDeath,
			DroppedAfterDeath,
		};
	}

	void CoroCallbackTest::testOuterCallbackDoesntResumeCancelledAwaiter_data ()
	{
		QTest::addColumn<Completion> ("completion");
		QTest::addColumn<bool> ("taskKept");

		QTest::newRow ("invoked before death, task kept") << Completion::InvokedBeforeDeath << true;
		QTest::newRow ("invoked before death, task dropped") << Completion::InvokedBeforeDeath << false;
		QTest::newRow ("invoked after death, task kept") << Completion::InvokedAfterDeath << true;
		QTest::newRow ("invoked after death, task dropped") << Completion::InvokedAfterDeath << false;
		QTest::newRow ("dropped after death, task kept") << Completion::DroppedAfterDeath << true;
		QTest::newRow ("dropped after death, task dropped") << Completion::DroppedAfterDeath << false;
	}

	void CoroCallbackTest::testOuterCallbackDoesntResumeCancelledAwaiter ()
	{
		QFETCH (const Completion, completion);
		QFETCH (const bool, taskKept);

		Callback<int> cb;
		std::function<void (int)> callback { cb.GetCallback () };
		auto context = std::make_unique<CoroContext> ();
		int resumes = 0;
		std::optional task { Doomed (*context, cb, resumes) };

		if (completion == Completion::InvokedBeforeDeath)
			callback (42);
		context.reset ();
		if (!taskKept)
			task.reset ();

		if (completion == Completion::InvokedAfterDeath)
			callback (42);
		callback = nullptr;

		QTest::qWait (10);
		QCOMPARE (resumes, 0);
		if (task)
			QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (*task));
	}

	namespace
	{
		Task<std::optional<int>> Parent (CoroContext& childContext,
				std::function<void (int)>& callback, std::function<void ()>& proceed, int& childResumes)
		{
			Callback<int> cb;
			callback = cb.GetCallback ();

			try
			{
				co_await Doomed (childContext, cb, childResumes);
			}
			catch (const ContextDeadException&)
			{
			}

			Callback<> gate;
			proceed = gate.GetCallback ();
			co_await gate;

			co_return co_await cb;
		}
	}

	void CoroCallbackTest::testCancelledChildLeavesResultToParent ()
	{
		std::function<void (int)> callback;
		std::function<void ()> proceed;
		auto childContext = std::make_unique<CoroContext> ();
		int childResumes = 0;
		auto parent = Parent (*childContext, callback, proceed, childResumes);

		childContext.reset ();
		callback (42);
		proceed ();

		const auto result = GetTaskResult (parent);
		QVERIFY (result.has_value ());
		QCOMPARE (*result, 42);
		QCOMPARE (childResumes, 0);
	}

	void CoroCallbackTest::testCallbackDestroyedWhileAwaited ()
	{
		auto cb = std::make_unique<Callback<int>> ();
		std::function<void (int)> callback { cb->GetCallback () };
		auto context = std::make_unique<CoroContext> ();
		int resumes = 0;
		auto task = Doomed (*context, *cb, resumes);

		cb.reset ();
		callback (42);
		callback = nullptr;
		QTest::qWait (10);
		QCOMPARE (resumes, 0);

		context.reset ();
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (task));
		QCOMPARE (resumes, 0);
	}
}
