/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "futurestest.h"
#include <QEventLoop>
#include <QtTest>
#include <futures.h>
#include "common.h"

QTEST_MAIN (LC::Util::FuturesTest)

namespace LC::Util
{
	void FuturesTest::testSequencer ()
	{
		QEventLoop loop;
		int res = 0;
		Sequence (nullptr, MkWaiter () (25))
			.Then (MkWaiter ())
			.Then (MkWaiter ())
			.Then (MkWaiter ())
			.Then ([&loop, &res] (int cnt)
					{
						res = cnt;
						loop.quit ();
					});

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (res, 400);
	}

	void FuturesTest::testHeterogeneousTypes ()
	{
		struct Bar {};
		struct Baz {};

		QEventLoop loop;
		bool executed = false;
		Sequence (nullptr, MkWaiter () (50)) >>
				[] (int) { return MakeReadyFuture<Bar> ({}); } >>
				[] (Bar) { return MakeReadyFuture<Baz> ({}); } >>
				[&executed, &loop] (Baz)
				{
					executed = true;
					loop.quit ();
				};

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (executed, true);
	}

	void FuturesTest::testDestruction ()
	{
		QEventLoop loop;
		bool executed = false;

		{
			QObject obj;
			Sequence (&obj, MakeReadyFuture (0)) >>
					[&executed, &loop] (int)
					{
						executed = true;
						loop.quit ();
					};
		}

		QTimer::singleShot (100, &loop, SLOT (quit ()));

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (executed, false);
	}

	void FuturesTest::testDestructionHandler ()
	{
		const auto finished = 1;
		const auto destructed = 2;

		QEventLoop loop;
		bool executed = false;
		int value = 0;

		QFuture<int> future;
		{
			QObject obj;
			future = Sequence (&obj, MkWaiter () (100))
					.DestructionValue ([] { return destructed; }) >>
					[=] (int) { return MakeReadyFuture (finished); };
		}
		Sequence (nullptr, future) >>
				[&executed, &value, &loop] (int val)
				{
					value = val;
					executed = true;
					loop.quit ();
				};

		QTimer::singleShot (10, &loop, SLOT (quit ()));

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (executed, true);
		QCOMPARE (value, destructed);
	}

	void FuturesTest::testNoDestrHandler ()
	{
		struct Bar {};
		struct Baz {};

		QEventLoop loop;
		bool executed = false;
		Sequence (nullptr, MkWaiter () (50))
				.DestructionValue ([&executed] { executed = true; }) >>
				[] (int) { return MakeReadyFuture<Bar> ({}); } >>
				[] (Bar) { return MakeReadyFuture<Baz> ({}); } >>
				[&loop] (Baz) { loop.quit (); };

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (executed, false);
	}

	void FuturesTest::testNoDestrHandlerSetBuildable ()
	{
		const auto finished = 1;

		QEventLoop loop;
		bool executed = false;
		int value = 0;

		QFuture<int> future = Sequence (nullptr, MkWaiter () (10)) >>
				[=] (int) { return MakeReadyFuture (finished); };
		Sequence (nullptr, future) >>
				[&executed, &value, &loop] (int val)
				{
					value = val;
					executed = true;
					loop.quit ();
				};

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (executed, true);
		QCOMPARE (value, finished);
	}

	void FuturesTest::testMulti ()
	{
		QEventLoop loop;

		QFutureInterface<int> iface;

		int count = 0;
		int sum = 0;
		Sequence (nullptr, iface.future ())
				.MultipleResults ([&] (int sub)
						{
							sum += sub;
							++count;
						},
						[&] { loop.quit (); });

		iface.reportStarted ();
		iface.setExpectedResultCount (3);
		while (iface.resultCount () < iface.expectedResultCount ())
			iface.reportResult (iface.resultCount () + 1, iface.resultCount ());
		iface.reportFinished ();

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (count, 3);
		QCOMPARE (sum, 6);
	}

	void FuturesTest::testMultiRange ()
	{
		QEventLoop loop;

		QFutureInterface<int> iface;

		int count = 0;
		int sum = 0;
		Sequence (nullptr, iface.future ())
				.MultipleResults ([&] (int sub)
						{
							sum += sub;
							++count;
						},
						[&] { loop.quit (); });

		iface.reportStarted ();
		iface.setProgressRange (0, 2);
		iface.reportResult (1, 0);
		iface.reportResult (2, 1);
		iface.reportResult (3, 2);
		iface.reportFinished ();

		loop.exec ();

		QCoreApplication::processEvents ();

		QCOMPARE (count, 3);
		QCOMPARE (sum, 6);
	}
}
