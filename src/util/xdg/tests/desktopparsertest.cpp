/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "desktopparsertest.h"
#include <QtTest>
#include <QtDebug>
#include <desktopparser.h>

QTEST_APPLESS_MAIN (LC::Util::XDG::DesktopParserTest)

template<typename K, typename V>
char* toString (const QHash<K, V>& hash)
{
	QString str;
	QDebug dbg { &str };
	dbg << hash;
	return qstrdup (str.toUtf8 ().constData ());
}

namespace LC::Util::XDG
{
	DesktopParser::LangValue_t SingleValue (const QString& val)
	{
		return { { {}, { val } } };
	}

	void DesktopParserTest::testBasicFile ()
	{
		DesktopParser p;
		const auto& res = p (R"(
[Desktop Entry]
Name=XSession
Comment=This session logs you into your custom Xsession
Icon=
)");
		const DesktopParser::Result_t expected
		{
			{
				"Desktop Entry",
				{
					{ "Name", SingleValue ("XSession") },
					{ "Comment", SingleValue ("This session logs you into your custom Xsession") },
					{ "Icon", SingleValue ("") },
				}
			}
		};
		QCOMPARE (res, expected);
	}
}
