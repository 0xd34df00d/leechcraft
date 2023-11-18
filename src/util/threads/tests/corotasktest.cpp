/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "corotasktest.h"
#include <QtTest>
#include <coro.h>
#include <coro/context.h>
#include <coro/either.h>
#include <coro/getresult.h>
#include <coro/inparallel.h>
#include <coro/networkresult.h>
#include <coro/throttle.h>
#include <util/sll/qtutil.h>

QTEST_GUILESS_MAIN (LC::Util::CoroTaskTest)

using namespace std::chrono_literals;

namespace LC::Util
{
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
			co_await 50ms;
			co_await Precisely { 10ms };
			co_return 42;
		} ();

		auto result = GetTaskResult (task);
		QCOMPARE (result, 42);
		QVERIFY (timer.elapsed () > 50);
	}

	void CoroTaskTest::testTaskDestr ()
	{
		bool continued = false;

		[&] () -> Task<void>
		{
			co_await 10ms;
			continued = true;
		} ();

		QTRY_VERIFY_WITH_TIMEOUT (continued, 20);
	}

	namespace
	{
		// almost the Public Morozov pattern
		class MockReply : public QNetworkReply
		{
			QBuffer Buffer_;
		public:
			using QNetworkReply::QNetworkReply;

			using QNetworkReply::setAttribute;
			using QNetworkReply::setError;
			using QNetworkReply::setFinished;
			using QNetworkReply::setHeader;
			using QNetworkReply::setOperation;
			using QNetworkReply::setRawHeader;
			using QNetworkReply::setRequest;
			using QNetworkReply::setUrl;

			void SetData (const QByteArray& data)
			{
				Buffer_.setData (data);
				Buffer_.open (QIODevice::ReadOnly);
				open (QIODevice::ReadOnly);
			}
		protected:
			qint64 readData (char *data, qint64 maxSize) override
			{
				return Buffer_.read (data, maxSize);
			}

			void abort () override
			{
			}
		};

		class MockNAM : public QNetworkAccessManager
		{
			std::unique_ptr<MockReply> Reply_;
		public:
			explicit MockNAM (std::unique_ptr<MockReply> reply)
			: Reply_ { std::move (reply) }
			{
			}

			MockReply& GetReply ()
			{
				return *Reply_;
			}
		protected:
			QNetworkReply* createRequest (Operation op, const QNetworkRequest& req, QIODevice*) override
			{
				Reply_->setUrl (req.url ());
				Reply_->setOperation (op);
				Reply_->setRequest (req);
				return Reply_.get ();
			}
		};

		auto MkSuccessfulReply (const QByteArray& data)
		{
			auto reply = std::make_unique<MockReply> ();
			reply->setAttribute (QNetworkRequest::HttpStatusCodeAttribute, 200);
			reply->SetData (data);
			return reply;
		}

		auto MkErrorReply ()
		{
			auto reply = std::make_unique<MockReply> ();
			reply->setAttribute (QNetworkRequest::HttpStatusCodeAttribute, 404);
			reply->setError (QNetworkReply::NetworkError::ContentAccessDenied, "well, 404!"_qs);
			return reply;
		}

		void TestGoodReply (auto finishMarker)
		{
			const QByteArray data { "this is some test data" };
			MockNAM nam { MkSuccessfulReply (data) };
			finishMarker (nam.GetReply ());

			auto task = [&nam] () -> Task<QByteArray>
			{
				auto reply = co_await *nam.get (QNetworkRequest { QUrl { "http://example.com/foo.txt"_qs } });
				co_return reply.GetReplyData ();
			} ();

			auto result = GetTaskResult (task);
			QCOMPARE (result, data);
		}

		void TestBadReply (auto finishMarker)
		{
			MockNAM nam { MkErrorReply () };
			finishMarker (nam.GetReply ());

			auto task = [&nam] () -> Task<QByteArray>
			{
				auto reply = co_await *nam.get (QNetworkRequest { QUrl { "http://example.com/foo.txt"_qs } });
				co_return reply.GetReplyData ();
			} ();

			QVERIFY_EXCEPTION_THROWN (GetTaskResult (task), LC::Util::NetworkReplyErrorException);
		}

		void ImmediateFinishMarker (MockReply& reply)
		{
			reply.setFinished (true);
		}

		void DelayedFinishMarker (MockReply& reply)
		{
			QTimer::singleShot (10ms,
					[&]
					{
						reply.setFinished (true);
						emit reply.finished ();
					});
		}
	}

	void CoroTaskTest::testNetworkReplyGoodNoWait ()
	{
		TestGoodReply (&ImmediateFinishMarker);
	}

	void CoroTaskTest::testNetworkReplyGoodWait ()
	{
		TestGoodReply (&DelayedFinishMarker);
	}

	void CoroTaskTest::testNetworkReplyBadNoWait ()
	{
		TestBadReply (&ImmediateFinishMarker);
	}

	void CoroTaskTest::testNetworkReplyBadWait ()
	{
		TestBadReply (&DelayedFinishMarker);
	}

	void CoroTaskTest::testContextDestrBeforeFinish ()
	{
		auto context = std::make_unique<QObject> ();
		auto task = [] (QObject *context) -> Task<int, ContextExtensions>
		{
			co_await AddContextObject { *context };
			co_await 10ms;
			co_return context->children ().size ();
		} (&*context);
		context.reset ();

		QVERIFY_EXCEPTION_THROWN (GetTaskResult (task), LC::Util::ContextDeadException);
	}

	void CoroTaskTest::testContextDestrAfterFinish ()
	{
		auto context = std::make_unique<QObject> ();
		auto task = [] (QObject *context) -> Task<int, ContextExtensions>
		{
			co_await AddContextObject { *context };
			co_await 10ms;
			co_return context->children ().size ();
		} (&*context);

		QCOMPARE (GetTaskResult (task), 0);
	}

	void CoroTaskTest::testWaitMany ()
	{
		constexpr auto max = 100;
		auto mkTask = [] (int index) -> Task<int>
		{
			co_await Precisely { std::chrono::milliseconds { max - index } };
			co_return index;
		};

		QElapsedTimer timer;
		timer.start ();
		QVector<Task<int>> tasks;
		QVector<int> expected;
		for (int i = 0; i < max; ++i)
		{
			tasks << mkTask (i);
			expected << i;
		}
		const auto creationElapsed = timer.elapsed ();

		timer.restart ();
		auto result = GetTaskResult (InParallel (std::move (tasks)));
		const auto executionElapsed = timer.elapsed ();

		QCOMPARE (result, expected);
		QVERIFY (creationElapsed < 1);
		constexpr auto tolerance = 1.05;
		QVERIFY (executionElapsed < max * tolerance);
	}

	void CoroTaskTest::testWaitManyTuple ()
	{
		auto mkTask = [] (int delay) -> Task<int>
		{
			co_await Precisely { std::chrono::milliseconds { delay } };
			co_return delay;
		};

		QElapsedTimer timer;
		timer.start ();
		auto result = GetTaskResult (InParallel (mkTask (10), mkTask (9), mkTask (2), mkTask (1)));
		const auto executionElapsed = timer.elapsed ();

		QCOMPARE (result, (std::tuple { 10, 9, 2, 1 }));
		constexpr auto tolerance = 1.05;
		QVERIFY (executionElapsed < 10 * tolerance);
	}

	void CoroTaskTest::testEither ()
	{
		using Result_t = Either<QString, bool>;

		auto earlyFailing = [] () -> Task<Result_t>
		{
			const auto theInt = co_await Either<QString, int>::Left ("meh");
			co_await 10ms;
			co_return Result_t::Right (theInt > 420);
		} ();
		QCOMPARE (GetTaskResult (earlyFailing), Result_t::Left ("meh"));

		auto successful = [] () -> Task<Result_t>
		{
			const auto theInt = co_await Either<QString, int>::Right (42);
			co_await 10ms;
			co_return Result_t::Right (theInt > 420);
		} ();
		QCOMPARE (GetTaskResult (successful), Result_t::Right (false));
	}

	void CoroTaskTest::testThrottleSameCoro ()
	{
		Throttle t { 10ms };
		constexpr auto count = 10;

		QElapsedTimer timer;
		timer.start ();
		auto task = [] (auto& t) -> Task<int>
		{
			int result = 0;
			for (int i = 0; i < count; ++i)
			{
				co_await t;
				result += i;
			}
			co_return result;
		} (t);
		const auto result = GetTaskResult (task);
		const auto time = timer.elapsed ();

		QCOMPARE (result, count * (count - 1) / 2);
		QVERIFY (time >= count * t.GetInterval ().count ());
	}

	void CoroTaskTest::testThrottleSameCoroSlow ()
	{
		Throttle t { 10ms };
		constexpr auto count = 10;

		QElapsedTimer timer;
		timer.start ();
		auto task = [] (auto& t) -> Task<void>
		{
			for (int i = 0; i < count; ++i)
			{
				co_await t;
				if (i != count - 1)
					co_await Precisely { 9ms };
			}
		} (t);
		GetTaskResult (task);
		const auto time = timer.elapsed ();

		const auto expectedMinTime = count * t.GetInterval ().count ();
		QVERIFY (time >= expectedMinTime);
		QVERIFY (time - expectedMinTime <= expectedMinTime * 0.05);
	}

	void CoroTaskTest::testThrottleManyCoros ()
	{
		Throttle t { 1ms, Qt::TimerType::PreciseTimer };
		constexpr auto count = 10;

		QElapsedTimer timer;
		timer.start ();
		auto mkTask = [] (auto& t) -> Task<void>
		{
			for (int i = 0; i < count; ++i)
				co_await t;
		};
		QVector tasks { mkTask (t), mkTask (t), mkTask (t) };
		for (auto& task : tasks)
			GetTaskResult (task);
		const auto time = timer.elapsed ();

		QVERIFY (time >= count * tasks.size () * t.GetInterval ().count ());
	}
}
