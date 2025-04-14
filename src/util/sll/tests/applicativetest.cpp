/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "applicativetest.h"
#include <QtTest>
#include <applicative.h>

QTEST_MAIN (LC::Util::ApplicativeTest)

namespace LC
{
namespace Util
{
	void ApplicativeTest::testBoostOptionalPure ()
	{
		const auto& pure = Pure<std::optional> (2);
		QCOMPARE (pure, std::optional<int> { 2 });
	}

	void ApplicativeTest::testBoostOptionalGSL ()
	{
		const auto& pure = Pure<std::optional> ([] (int a) { return ++a; });
		const auto& app = GSL (pure, Pure<std::optional> (2));
		QCOMPARE (app, std::optional<int> { 3 });
	}
}
}
