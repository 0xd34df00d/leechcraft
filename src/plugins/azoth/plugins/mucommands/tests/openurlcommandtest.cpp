/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "openurlcommandtest.h"
#include <QtTest>
#include <QtDebug>
#include "openurlcommand.cpp"
#include "testutil.h"

QTEST_APPLESS_MAIN (LC::Azoth::MuCommands::OpenUrlCommandTest)

template<typename T>
QDebug operator<< (QDebug dbg, const std::optional<T>& opt)
{
	if (opt)
		dbg.nospace () << *opt;
	else
		dbg.nospace () << "<nothing>";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::Azoth::MuCommands::UrlRange& r)
{
	dbg.nospace () << "UrlRange { start: "
			<< r.Start_
			<< "; end: "
			<< r.End_
			<< " }";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::Azoth::MuCommands::UrlComposite& c)
{
	dbg.nospace () << "UrlComposite { "
			<< LC::Azoth::MuCommands::PrintVisitor {} (c.Range_).toUtf8 ()
			<< "; rx: `"
			<< boost::get_optional_value_or (c.Pat_, "").c_str ()
			<< "` }";
	return dbg.space ();
}

namespace QTest
{
	template<>
	char* toString (const LC::Azoth::MuCommands::OpenUrlParams_t& var)
	{
		return LC::Azoth::MuCommands::PrintVar (var);
	}
}

namespace LC
{
namespace Azoth
{
namespace MuCommands
{
	bool operator== (const UrlRange& r1, const UrlRange& r2)
	{
		return r1.Start_ == r2.Start_ &&
				r1.End_ == r2.End_;
	}

	bool operator== (const UrlComposite& r1, const UrlComposite& r2)
	{
		return r1.Pat_ == r2.Pat_ &&
				r1.Range_ == r2.Range_;
	}

	bool operator== (const SinceLast&, const SinceLast&)
	{
		return true;
	}

	bool operator== (const JustLast&, const JustLast&)
	{
		return true;
	}

	bool operator== (const All&, const All&)
	{
		return true;
	}

	void OpenUrlCommandTest::parseSinceLast ()
	{
		const QString command = "/openurl last";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { SinceLast {}, {} } }));
	}

	void OpenUrlCommandTest::parseLast ()
	{
		const QString command = "/openurl";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { JustLast {} }));
	}

	void OpenUrlCommandTest::parseAll ()
	{
		const QString command = "/openurl *";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { All {}, {} } }));
	}

	void OpenUrlCommandTest::parseByIndex ()
	{
		const QString command = "/openurl 3";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlIndex_t { 3 } }));
	}

	void OpenUrlCommandTest::parseByRange ()
	{
		const QString command = "/openurl 3:10";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { 3, 10 }, {} } }));
	}

	void OpenUrlCommandTest::parseByLeftOpenRange ()
	{
		const QString command = "/openurl :10";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { {}, 10 }, {} } }));
	}

	void OpenUrlCommandTest::parseByRightOpenRange ()
	{
		const QString command = "/openurl 3:";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { 3, {} }, {} } }));
	}

	void OpenUrlCommandTest::parseByFullOpenRange ()
	{
		const QString command = "/openurl :";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { {}, {} }, {} } }));
	}

	void OpenUrlCommandTest::parseByRx ()
	{
		const QString command = "/openurl rx ^[a-c]$";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { RegExpStr_t { "^[a-c]$" } }));
	}

	void OpenUrlCommandTest::parseByRxSpaces ()
	{
		const QString command = "/openurl rx ^[a-c] space smth$";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { RegExpStr_t { "^[a-c] space smth$" } }));
	}

	void OpenUrlCommandTest::parseByRxRanged ()
	{
		const QString command = "/openurl 3:4 rx ^[a-c]$";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { 3, 4 }, std::string { "^[a-c]$" } } }));
	}

	void OpenUrlCommandTest::parseByRxSpacesRanged ()
	{
		const QString command = "/openurl 3:4 rx ^[a-c] space smth$";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { 3, 4 }, std::string { "^[a-c] space smth$" } } }));
	}

	void OpenUrlCommandTest::parseByRxLast ()
	{
		const QString command = "/openurl last rx ^[a-c] space smth$";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { SinceLast {}, std::string { "^[a-c] space smth$" } } }));
	}

	void OpenUrlCommandTest::failLeftover()
	{
		const QString command = "/openurl without any rx";
		bool caught = false;
		try
		{
			ParseCommand (command);
		}
		catch (const ParseError&)
		{
			caught = true;
		}
		QCOMPARE (caught, caught);
	}

	void OpenUrlCommandTest::failLeftoverLast ()
	{
		const QString command = "/openurl last without any rx";
		bool caught = false;
		try
		{
			ParseCommand (command);
		}
		catch (const ParseError&)
		{
			caught = true;
		}
		QCOMPARE (caught, caught);
	}
}
}
}
