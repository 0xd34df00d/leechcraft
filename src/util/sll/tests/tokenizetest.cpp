/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tokenizetest.h"
#include <QtTest>
#include <tokenize.h>

QTEST_APPLESS_MAIN (LC::Util::TokenizeTest)

namespace LC::Util
{
	using QSVV = QList<QStringView>;

	auto TokenizeFwd (QStringView str)
	{
		Tokenize t { str, '.' };
		QSVV tokenized { t.begin (), t.end () };
		QCOMPARE (tokenized, str.split ('.'));
	}

	void TokenizeTest::testForwardEmpty ()
	{
		// not ideal, but it simplifies the implementation a lot
		TokenizeFwd ({});
	}

	void TokenizeTest::testForwardSimple ()
	{
		TokenizeFwd (u"some.test.string");
	}

	void TokenizeTest::testForwardWithEmpty ()
	{
		TokenizeFwd (u"some..test.string");
	}

	void TokenizeTest::testForwardStartSep ()
	{
		TokenizeFwd (u"...some..test.string");
	}

	void TokenizeTest::testForwardEndSep ()
	{
		TokenizeFwd (u"some..test.string..");
	}

	void TokenizeTest::testForwardJustSeps ()
	{
		TokenizeFwd (u".");
		TokenizeFwd (u"..");
	}


	auto TokenizeRev (QStringView str)
	{
		Tokenize t { str, '.' };
		QSVV tokenized { t.rbegin (), t.rend () };

		auto reference = str.split ('.');
		std::reverse (reference.begin (), reference.end ());
		QCOMPARE (tokenized, reference);
	}

	auto Reversed (auto container)
	{
		std::reverse (container.begin (), container.end ());
		return container;
	}

	void TokenizeTest::testReverseEmpty ()
	{
		TokenizeRev ({});
	}

	void TokenizeTest::testReverseSimple ()
	{
		TokenizeRev (u"some.test.string");
	}

	void TokenizeTest::testReverseWithEmpty ()
	{
		TokenizeRev (u"some..test.string");
	}

	void TokenizeTest::testReverseStartSep ()
	{
		TokenizeRev (u"...some..test.string");
	}

	void TokenizeTest::testReverseEndSep ()
	{
		TokenizeRev (u"some..test.string..");
	}

	void TokenizeTest::testReverseJustSeps ()
	{
		TokenizeRev (u".");
		TokenizeRev (u"..");
	}
}
