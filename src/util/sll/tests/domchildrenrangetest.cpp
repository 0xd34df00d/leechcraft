/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "domchildrenrangetest.h"
#include <QDomDocument>
#include <QString>
#include <QtTest>
#include <domchildrenrange.h>

QTEST_MAIN (LC::Util::DomChildrenRangeTest)

namespace LC
{
namespace Util
{
	namespace
	{
		auto MakeDocument (const QString& str)
		{
			QDomDocument doc;
			doc.setContent (str);
			return doc.firstChildElement ("root");
		}
	}

	void DomChildrenRangeTest::testEmpty ()
	{
		const auto& parent = MakeDocument (R"(
				<root>
				</root>
				)");

		QStringList texts;
		for (const auto& elem : DomChildren (parent, "child"))
			texts << elem.text ();
		QCOMPARE (texts, QStringList {});
	}

	void DomChildrenRangeTest::testSingle ()
	{
		const auto& parent = MakeDocument (R"(
				<root>
					<child>foo</child>
				</root>
				)");

		QStringList texts;
		for (const auto& elem : DomChildren (parent, "child"))
			texts << elem.text ();
		QCOMPARE (texts, QStringList { "foo" });
	}

	void DomChildrenRangeTest::testMultiple ()
	{
		const auto& parent = MakeDocument (R"(
				<root>
					<child>foo</child>
					<child>bar</child>
					<child>baz</child>
				</root>
				)");

		QStringList texts;
		for (const auto& elem : DomChildren (parent, "child"))
			texts << elem.text ();
		QCOMPARE (texts, (QStringList { "foo", "bar", "baz" }));
	}
}
}
