/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pslhandlertest.h"
#include <QTest>
#include <util/sll/qtutil.h>
#include "pslhandler.cpp"

QTEST_APPLESS_MAIN (LC::Poshuku::CleanWeb::PslHandlerTest)

namespace LC::Poshuku::CleanWeb
{
	namespace
	{
		// taken from https://publicsuffix.org/list/
		const QStringView ExampleContents = uR"(
com

*.jp
// Hosts in .hokkaido.jp can't set cookies below level 4...
*.hokkaido.jp
*.tokyo.jp
// ...except hosts in pref.hokkaido.jp, which can set cookies at level 3.
!pref.hokkaido.jp
!metro.tokyo.jp
)";
	}

	void PslHandlerTest::testExampleLookups ()
	{
		const PslHandler handler { ExampleContents };

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

	void PslHandlerTest::testRegistrableDomain ()
	{
		const PslHandler handler { ExampleContents };

		auto domain = [&handler] (QStringView host) { return handler.GetRegistrableDomain (host); };
		QCOMPARE (domain (u"foo.com"), u"foo.com"_qs);
		QCOMPARE (domain (u"a.b.foo.com"), u"foo.com"_qs);
		QCOMPARE (domain (u"Foo.COM"), u"foo.com"_qs);
		QCOMPARE (domain (u"foo.com."), u"foo.com"_qs);

		QCOMPARE (domain (u"baz.foo.bar.jp"), u"foo.bar.jp"_qs);
		QCOMPARE (domain (u"a.b.c.bar.hokkaido.jp"), u"c.bar.hokkaido.jp"_qs);
		QCOMPARE (domain (u"a.pref.hokkaido.jp"), u"pref.hokkaido.jp"_qs);

		QCOMPARE (domain (u"bar.jp"), u"bar.jp"_qs);
		QCOMPARE (domain (u"com"), u"com"_qs);
		QCOMPARE (domain (u"localhost"), u"localhost"_qs);

		QVERIFY (domain (u"ads.foo.com") == domain (u"www.foo.com"));
		QVERIFY (domain (u"foo.com") != domain (u"bar.com"));
	}
}
