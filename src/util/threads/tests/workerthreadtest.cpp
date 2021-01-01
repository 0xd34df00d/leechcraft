/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "workerthreadtest.h"
#include <QtTest>
#include <workerthreadbase.h>

QTEST_MAIN (LC::Util::WorkerThreadTest)

namespace LC::Util
{
	struct Worker
	{
		QThread * const Main_;
		int& Val_;

		explicit Worker (int *val, QThread *main)
		: Main_ { main }
		, Val_ { *val }
		{
		}

		void Increment (int n)
		{
			qDebug () << QThread::currentThread ();
			QVERIFY (QThread::currentThread () != Main_);

			Val_ += n;
		}

		void Quit (QEventLoop& loop)
		{
			QTimer::singleShot (0, &loop, &QEventLoop::quit);
		}
	};

	void WorkerThreadTest::testWorkerThread ()
	{
		QEventLoop loop;
		auto spin = [&loop]
		{
			QTimer::singleShot (100, &loop, &QEventLoop::quit);
			loop.exec ();
		};

		qDebug () << Q_FUNC_INFO << QThread::currentThread ();

		int val = 0;
		WorkerThread<Worker> worker { &val, QThread::currentThread () };
		worker.SetAutoQuit (true);
		worker.SetQuitWait (1000);
		worker.start ();

		worker.SetPaused (true);
		worker.ScheduleImpl (&Worker::Increment, 1);
		worker.ScheduleImpl (&Worker::Increment, 2);
		worker.ScheduleImpl (&Worker::Increment, 3);

		spin ();
		QCOMPARE (val, 0);

		worker.SetPaused (false);

		spin ();
		QCOMPARE (val, 6);

		worker.ScheduleImpl (&Worker::Increment, 10);

		spin ();
		QCOMPARE (val, 16);
	}
}
