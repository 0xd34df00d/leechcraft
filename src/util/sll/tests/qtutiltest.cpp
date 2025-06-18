/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "qtutiltest.h"
#include <QtTest>
#include <qtutil.h>

QTEST_APPLESS_MAIN (LC::Util::QtUtilTest)

namespace LC::Util
{
	void QtUtilTest::testStringUDL ()
	{
		const auto& foo1 = "foo"_qs;
		const auto& bar = "bar"_qs;
		QCOMPARE (foo1, "foo");
		QCOMPARE (bar, "bar");

		auto foo2 = "foo"_qs;
		QCOMPARE (foo2, "foo");

		foo2.chop (1);
		QCOMPARE (foo2, "fo");
		QCOMPARE (foo1, "foo");
	}

	void QtUtilTest::testStringUDLBench ()
	{
		QFETCH (int, strInit);

		switch (strInit)
		{
		case 0:
			QBENCHMARK
			{
				const QString str { "foo" };
			}
			break;
		case 1:
			QBENCHMARK
			{
				const auto str = QStringLiteral ("foo");
			}
			break;
		case 2:
			QBENCHMARK
			{
				const auto str = "foo"_qs;
			}
			break;
		}
	}

	void QtUtilTest::testStringUDLBench_data ()
	{
		QTest::addColumn<int> ("strInit");
		QTest::newRow ("ctor") << 0;
		QTest::newRow ("QStringLiteral") << 1;
		QTest::newRow ("UDL") << 2;
	}

	void QtUtilTest::testHandleQVariant ()
	{
		{
			const auto v = HandleQVariant ("foo"_qs,
					[] (int) { return 0; },
					[] (const QString&) { return 1; },
					[] (const QByteArray&) { return 2; },
					[] { return 42; });
			QCOMPARE (v, 1);
		}

		{
			const auto v = HandleQVariant ("foo"_qs,
					[] (int n) { return QString::number (n); },
					[] (const QString& s) { return s; },
					[] (const QByteArray& ba) { return ba; },
					[] { return "default"_qs; });
			QCOMPARE (v, "foo"_qs);
		}

		{
			const auto v = HandleQVariant (42,
					[] (int n) { return QString::number (n); },
					[] (const QString& s) { return s; },
					[] (const QByteArray& ba) { return ba; },
					[] { return "default"_qs; });
			QCOMPARE (v, "42"_qs);
		}

		{
			const auto v = HandleQVariant (42ULL,
					[] (int n) { return QString::number (n); },
					[] (const QString& s) { return s; },
					[] (const QByteArray& ba) { return ba; },
					[] { return "default"_qs; });
			QCOMPARE (v, "default"_qs);
		}
	}
}
