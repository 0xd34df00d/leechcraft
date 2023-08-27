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
#include <coro/context.h>
#include <coro/networkresult.h>
#include <util/sll/qtutil.h>

QTEST_GUILESS_MAIN (LC::Util::CoroTaskTest)

using namespace std::chrono_literals;

namespace LC::Util
{
	namespace
	{
		template<typename T, template<typename> typename Extensions>
		T GetTaskResult (Task<T, Extensions> task)
		{
			constexpr bool isVoid = std::is_same_v<T, void>;
			std::conditional_t<isVoid, void*, T> result;

			std::exception_ptr exception;

			QEventLoop loop;
			bool done = false;
			[&] () -> Task<void>
			{
				try
				{
					if constexpr (isVoid)
						co_await task;
					else
						result = co_await task;
				}
				catch (...)
				{
					exception = std::current_exception ();
				}
				done = true;
				loop.quit ();
			} ();
			if (!done)
				loop.exec ();

			if (exception)
				std::rethrow_exception (exception);

			if constexpr (!isVoid)
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
}
