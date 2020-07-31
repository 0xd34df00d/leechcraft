/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scopeguardtest.h"
#include <QtTest>
#include "util.h"

QTEST_MAIN (LC::Util::ScopeGuardTest)

namespace LC
{
namespace Util
{
	void ScopeGuardTest::testBasicGuard ()
	{
		bool triggered = false;

		{
			const auto guard = MakeScopeGuard ([&triggered] { triggered = true; });
		}

		QCOMPARE (triggered, true);
	}

	void ScopeGuardTest::testAssignmentGuard ()
	{
		bool first = false;
		bool second = false;

		{
			DefaultScopeGuard guard = MakeScopeGuard ([&first] { first = true; });
			guard = MakeScopeGuard ([&second] { second = true; });
		}

		QCOMPARE (first, true);
		QCOMPARE (second, true);
	}
}
}
