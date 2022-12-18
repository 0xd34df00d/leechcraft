/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utils_test.h"
#include <QtTest>
#include <components/parsers/utils.h>

namespace LC::Aggregator::Parsers
{
	void UtilsTest::testUnescapeHTML ()
	{
		QCOMPARE (UnescapeHTML ("&quot;foo&quot;"), "\"foo\"");
		QCOMPARE (UnescapeHTML ("&euro;&euro;"), "€€");

		QCOMPARE (UnescapeHTML ("&#8217;"), "'");
		QCOMPARE (UnescapeHTML ("&#8217;&#37;&#40;&#8217;"), "'%('");
		QCOMPARE (UnescapeHTML ("&#8217&#37;&#40;&#8217"), "&#8217%(&#8217");
	}
}
