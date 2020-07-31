/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stlizetest.h"
#include <QtTest>
#include <qtutil.h>

QTEST_MAIN (LC::Util::StlizeTest)

namespace LC
{
namespace Util
{
	namespace
	{
		QMap<int, QString> GetSimpleMap ()
		{
			QMap<int, QString> someMap;
			someMap [0] = "aaa";
			someMap [1] = "bbb";
			someMap [2] = "ccc";
			return someMap;
		}
	}

	void StlizeTest::testConst ()
	{
		const auto& map = GetSimpleMap ();

		QStringList list;
		for (const auto& pair : Util::Stlize (map))
			list << pair.second;

		QCOMPARE (list, (QStringList { "aaa", "bbb", "ccc" }));
	}

	void StlizeTest::testNonConst ()
	{
		auto map = GetSimpleMap ();

		QStringList list;
		for (const auto& pair : Util::Stlize (map))
			list << pair.second;

		QCOMPARE (list, (QStringList { "aaa", "bbb", "ccc" }));
	}

	void StlizeTest::testNonConstModify ()
	{
		auto getMap = []
		{
			QMap<int, QString> someMap;
			someMap [0] = "aaa";
			someMap [1] = "bbb";
			someMap [2] = "ccc";
			return someMap;
		};

		auto map = getMap ();

		QStringList list;
		for (const auto& pair : Util::Stlize (map))
		{
			list << pair.second;
			pair.second.clear ();
		}

		QCOMPARE (list, (QStringList { "aaa", "bbb", "ccc" }));
		QCOMPARE (true, (std::all_of (map.begin (), map.end (), [] (const QString& str) { return str.isEmpty (); })));
	}

	void StlizeTest::testRvalue ()
	{
		QStringList list;
		for (const auto& pair : Util::Stlize (GetSimpleMap ()))
			list << pair.second;

		QCOMPARE (list, (QStringList { "aaa", "bbb", "ccc" }));
	}

	void StlizeTest::testAnyOf ()
	{
		const auto& stlized = Util::Stlize (GetSimpleMap ());
		const bool hasBbb = std::any_of (stlized.begin (), stlized.end (),
				[] (const auto& pair) { return pair.second == "bbb"; });

		QCOMPARE (hasBbb, true);
	}

	namespace
	{
		QMap<int, int> GetBigMap ()
		{
			QMap<int, int> map;
			for (int i = 0; i < 1500000; ++i)
				map [i] = i * 2;
			return map;
		}
	}

	void StlizeTest::benchmarkPlain ()
	{
		const auto& map = GetBigMap ();
		QBENCHMARK {
			volatile int sum = 0;
			for (auto value : map)
				sum = sum + value;
		}
	}

	void StlizeTest::benchmarkStlized ()
	{
		const auto& map = GetBigMap ();
		QBENCHMARK {
			volatile int sum = 0;
			for (const auto& pair : Util::Stlize (map))
				sum = sum + pair.second;
		}
	}
}
}

