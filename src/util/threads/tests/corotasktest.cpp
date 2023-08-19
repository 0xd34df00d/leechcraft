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
#include <coro/networkresult.h>

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
	}
}
