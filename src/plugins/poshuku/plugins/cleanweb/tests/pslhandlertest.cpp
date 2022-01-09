/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pslhandlertest.h"
#include <QtTest>
#include "pslhandler.cpp"

QTEST_APPLESS_MAIN (LC::Poshuku::CleanWeb::PslHandlerTest)

namespace LC::Poshuku::CleanWeb
{
	// taken from https://publicsuffix.org/list/
	void PslHandlerTest::testExampleLookups ()
	{
		const QStringView exampleContents = uR"(
com

*.jp
// Hosts in .hokkaido.jp can't set cookies below level 4...
*.hokkaido.jp
*.tokyo.jp
// ...except hosts in pref.hokkaido.jp, which can set cookies at level 3.
!pref.hokkaido.jp
!metro.tokyo.jp
)";

		const PslHandler handler { exampleContents };

		auto find = [&handler] (QStringView host) { return handler.GetTldCountHost (host); };
		QCOMPARE (find (u"foo.com"), 1);

		QCOMPARE (find (u"foo.bar.jp"), 2);
		QCOMPARE (find (u"bar.jp"), 2);

		QCOMPARE (find (u"foo.bar.hokkaido.jp"), 3);
		QCOMPARE (find (u"bar.hokkaido.jp"), 3);

		QCOMPARE (find (u"pref.hokkaido.jp"), 2);
	}

	void PslHandlerTest::testEmpty ()
	{
		const PslHandler handler { {} };

		QCOMPARE (handler.GetTldCountHost (u"foo.com"), 1);
	}
}
