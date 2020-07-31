/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "preludetest.h"
#include <QtTest>
#include <prelude.h>
#include <qtutil.h>

QTEST_MAIN (LC::Util::PreludeTest)

namespace LC
{
namespace Util
{
	namespace
	{
		QMap<int, QString> GetSimpleMap ()
		{
			return { { 0, "aaa" }, { 1, "bbb" }, { 2, "ccc" }};
		}
	}

	void PreludeTest::testMapList ()
	{
		QList<int> list { 1, 2, 3 };
		const auto& otherList = Map (list, [] (int v) { return QString::number (v); });

		QCOMPARE (otherList, (QStringList { "1", "2", "3" }));
	}

	void PreludeTest::testMapMap ()
	{
		const auto& map = GetSimpleMap ();
		const auto& otherList = Map (map, [] (const QString& v) { return v.size (); });

		QCOMPARE (otherList, (QList<int> { 3, 3, 3 }));
	}

	void PreludeTest::testMapStringList ()
	{
		const QStringList list { "aaa", "bbb", "ccc" };
		const auto& result = Map (list, [] (const QString& s) { return s.size (); });

		QCOMPARE (result, (QList<int> { 3, 3, 3 }));
	}

	void PreludeTest::testMapMapStlized ()
	{
		const auto& map = GetSimpleMap ();
		const auto& list = Map (Stlize (map), [] (const std::pair<int, QString>& pair) { return pair.second; });

		QCOMPARE (list, QStringList { map.values () });
	}

	void PreludeTest::testMapMember ()
	{
		struct Test
		{
			int m_a;
			int m_b;
		};

		const QList<Test> tests { { 1, 2 }, { 2, 4 }, { 3, 6 } };
		const auto& ints = Map (tests, &Test::m_a);

		QCOMPARE (ints, (QList<int> { 1, 2, 3 }));
	}

	void PreludeTest::testMapMemberFunction ()
	{
		struct Test
		{
			int m_a;

			int GetA () const
			{
				return m_a;
			}
		};

		const QList<Test> tests { { 1 }, { 2 }, { 3 } };
		const auto& ints = Map (tests, &Test::GetA);

		QCOMPARE (ints, (QList<int> { 1, 2, 3 }));
	}

	void PreludeTest::testConcatLists ()
	{
		QList<QList<int>> listOfLists
		{
			{ 1, 2 },
			{ 3 },
			{ 4, 5, 6 }
		};

		const auto& concat = Concat (listOfLists);
		QCOMPARE (concat, (QList<int> { 1, 2, 3, 4, 5, 6 }));
	}

	void PreludeTest::testConcatSets ()
	{
		QList<QSet<int>> listOfSets
		{
			{ 1, 2, 3 },
			{ 3, 4 },
			{ 4, 5, 6 }
		};

		const auto& concat = Concat (listOfSets);
		QCOMPARE (concat, (QSet<int> { 1, 2, 3, 4, 5, 6 }));
	}
}
}
