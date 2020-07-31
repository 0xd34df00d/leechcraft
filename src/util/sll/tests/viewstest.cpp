/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewstest.h"
#include <QtTest>
#include <views.h>
#include <prelude.h>

QTEST_MAIN (LC::Util::ViewsTest)

namespace LC
{
namespace Util
{
	void ViewsTest::testZipView ()
	{
		QList<int> ints { 1, 2, 3 };
		QList<QString> strings { "a", "b", "c" };

		QList<QPair<int, QString>> pairs;
		for (const auto& pair : Views::Zip (ints, strings))
			pairs << pair;

		QCOMPARE (pairs, (Zip (ints, strings)));
	}

	void ViewsTest::testZipViewDifferentLengths ()
	{
		QList<int> ints { 1, 2, 3, 4, 5 };
		QList<QString> strings { "a", "b", "c" };

		QList<QPair<int, QString>> pairs;
		for (const auto& pair : Views::Zip (ints, strings))
			pairs << pair;

		QCOMPARE (pairs, (Zip (ints, strings)));
	}
}
}
