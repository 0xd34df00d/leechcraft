/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utils_test.h"
#include <QDomDocument>
#include <QtTest>
#include <components/parsers/utils.h>

namespace LC::Aggregator::Parsers
{
	namespace
	{
		QDomDocument ParseEntry (const QByteArray& contentElem)
		{
			QDomDocument doc;
			const auto result = doc.setContent ("<feed xmlns=\"http://www.w3.org/2005/Atom\">" + contentElem + "</feed>",
					QDomDocument::ParseOption::UseNamespaceProcessing);
			if (!result)
				qFatal () << "cannot parse test document:" << result.errorMessage;
			return doc;
		}

		QDomElement GetContent (const QDomDocument& doc)
		{
			return doc.documentElement ().firstChildElement ("content");
		}
	}

	void UtilsTest::testParseContentXhtml ()
	{
		const auto& doc = ParseEntry (R"(<content type="xhtml"><div xmlns="http://www.w3.org/1999/xhtml">Hello <b>bold</b> &amp; <a href="post/1">link</a><p>para</p></div></content>)");
		QCOMPARE (Atom::ParseContent (GetContent (doc)), R"(Hello <b>bold</b> &amp; <a href="post/1">link</a><p>para</p>)");
		QCOMPARE (Atom::ParseEscapeAware (GetContent (doc)), "Hello bold & linkpara");

		const auto& noDiv = ParseEntry (R"(<content type="xhtml">Hello <b xmlns="http://www.w3.org/1999/xhtml">bold</b></content>)");
		QCOMPARE (Atom::ParseContent (GetContent (noDiv)), "Hello <b>bold</b>");
	}

	void UtilsTest::testParseContentEscaped ()
	{
		const auto& html = ParseEntry (R"(<content type="html">Hello &lt;b&gt;bold&lt;/b&gt; &amp;amp; more</content>)");
		QCOMPARE (Atom::ParseContent (GetContent (html)), "Hello <b>bold</b> &amp; more");

		const auto& text = ParseEntry (R"(<content>Hello &lt;b&gt;</content>)");
		QCOMPARE (Atom::ParseContent (GetContent (text)), "Hello <b>");
	}

	void UtilsTest::testUnescapeHTML ()
	{
		QCOMPARE (UnescapeHTML ("&quot;foo&quot;"), "\"foo\"");
		QCOMPARE (UnescapeHTML ("&euro;&euro;"), "€€");

		QCOMPARE (UnescapeHTML ("&#8217;"), "'");
		QCOMPARE (UnescapeHTML ("&#8217;&#37;&#40;&#8217;"), "'%('");
		QCOMPARE (UnescapeHTML ("&#8217&#37;&#40;&#8217"), "&#8217%(&#8217");
	}
}
