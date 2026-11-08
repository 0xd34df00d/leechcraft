/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "contentdispositiontest.h"
#include <QTest>
#include <util/sll/qtutil.h>
#include "../contentdisposition.h"

QTEST_APPLESS_MAIN (LC::CSTP::ContentDispositionTest)

// Most cases come from the examples in RFC 6266 and from the tc2231 test suite at
// http://test.greenbytes.de/tech/tc2231/, which also says which malformed headers to ignore.

namespace LC::CSTP
{
	void ContentDispositionTest::testSuggestedFilename ()
	{
		QFETCH (QByteArray, header);
		QFETCH (QString, filename);

		const auto& suggested = GetSuggestedFilename (header);
		QVERIFY (suggested);
		QCOMPARE (*suggested, filename);
	}

	void ContentDispositionTest::testSuggestedFilename_data ()
	{
		QTest::addColumn<QByteArray> ("header");
		QTest::addColumn<QString> ("filename");

		QTest::newRow ("token") << "Attachment; filename=example.html"_qba << "example.html";
		QTest::newRow ("upper-case name") << R"(INLINE; FILENAME= "an example.html")"_qba << "an example.html";
		QTest::newRow ("filename* only") << "attachment; filename*= UTF-8''%e2%82%ac%20rates"_qba << "€ rates";
		QTest::newRow ("filename* wins") << R"(attachment; filename="EURO rates"; filename*=utf-8''%e2%82%ac%20rates)"_qba
				<< "€ rates";
		QTest::newRow ("language tag") << "attachment; filename*=UTF-8'en'%e2%82%ac%20rates"_qba << "€ rates";
		QTest::newRow ("ISO-8859-1") << "attachment; filename*=iso-8859-1''%A3%20rates"_qba << "£ rates";
		QTest::newRow ("mixed-case filename*") << "attachment; FileName*=UTF-8''mixed.txt"_qba << "mixed.txt";
		QTest::newRow ("space after filename*=") << "attachment; filename*= UTF-8''foo-%c3%a4.html"_qba << "foo-ä.html";
		QTest::newRow ("unknown charset") << R"(attachment; filename="fallback.txt"; filename*=koi8-r''%E1)"_qba
				<< "fallback.txt";
		QTest::newRow ("empty filename*") << R"(attachment; filename="plain.txt"; filename*=UTF-8'')"_qba << "plain.txt";
		QTest::newRow ("continuations")
				<< "attachment; filename*0*=ISO-8859-15''euro-sign%3d%a4; filename*=ISO-8859-1''currency-sign%3d%a4"_qba
				<< "currency-sign=¤";

		QTest::newRow ("token before another parameter") << "attachment; filename=report.pdf; size=1234"_qba << "report.pdf";
		QTest::newRow ("filename= in another value") << R"(attachment; title="see filename=evil.exe"; filename="ok.txt")"_qba
				<< "ok.txt";
		QTest::newRow ("semicolon in quotes") << R"(inline; filename="x;y.txt")"_qba << "x;y.txt";
		QTest::newRow ("escaped quote") << R"(attachment; filename="a\"b.txt")"_qba << R"(a"b.txt)";
		QTest::newRow ("escaped character") << R"(attachment; filename="f\oo.html")"_qba << "foo.html";
		QTest::newRow ("single quotes") << "attachment; filename='foo.bar'"_qba << "'foo.bar'";
		QTest::newRow ("spaces around =") << R"(attachment; filename = "spaced.txt")"_qba << "spaced.txt";
		QTest::newRow ("two filenames") << R"(attachment; filename="foo.html"; filename="bar.html")"_qba << "foo.html";
		QTest::newRow ("raw UTF-8") << R"(attachment; filename="café.txt")"_qba << "café.txt";

		QTest::newRow ("parent directories") << "attachment; filename=../../x.txt"_qba << "x.txt";
		QTest::newRow ("encoded slashes") << "attachment; filename*=UTF-8''..%2F..%2Fx.txt"_qba << "x.txt";
		QTest::newRow ("absolute path") << R"(attachment; filename="/etc/passwd-new")"_qba << "passwd-new";
	}

	void ContentDispositionTest::testNoSuggestion ()
	{
		QFETCH (QByteArray, header);

		if (const auto& suggested = GetSuggestedFilename (header))
			QFAIL (qPrintable ("suggested " + *suggested));
	}

	void ContentDispositionTest::testNoSuggestion_data ()
	{
		QTest::addColumn<QByteArray> ("header");

		QTest::newRow ("no header") << QByteArray {};
		QTest::newRow ("no parameters") << "attachment"_qba;
		QTest::newRow ("no disposition type") << R"(filename="x.txt")"_qba;
		QTest::newRow ("unterminated quote") << R"(attachment; filename="unterminated)"_qba;
		QTest::newRow ("junk after the quote") << R"(attachment; filename="foo.html".txt)"_qba;
		QTest::newRow ("quoted filename*") << R"(attachment; filename*="UTF-8''foo.html")"_qba;
		QTest::newRow ("no charset") << "attachment; filename*=''foo-%c3%a4-%e2%82%ac.html"_qba;
		QTest::newRow ("space before *") << "attachment; filename *=UTF-8''foo-%c3%a4.html"_qba;
		QTest::newRow ("missing single quote") << "attachment; filename*=UTF-8'foo-%c3%a4.html"_qba;

		QTest::newRow ("dot") << R"(attachment; filename=".")"_qba;
		QTest::newRow ("dot dot") << R"(attachment; filename="..")"_qba;
		QTest::newRow ("trailing slash") << R"(attachment; filename="../")"_qba;
		QTest::newRow ("ends in dot dot") << R"(attachment; filename="sub/..")"_qba;
		QTest::newRow ("encoded dot dot") << "attachment; filename*=UTF-8''%2E%2E"_qba;
	}
}
