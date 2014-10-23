/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "openurlcommandtest.h"
#include <QtTest>
#include <QtDebug>
#include "openurlcommand.cpp"
#include "testutil.h"

QTEST_MAIN (LeechCraft::Azoth::MuCommands::OpenUrlCommandTest)

template<typename T>
QDebug operator<< (QDebug dbg, const boost::optional<T>& opt)
{
	if (opt)
		dbg.nospace () << *opt;
	else
		dbg.nospace () << "<nothing>";
	return dbg.space ();
}

namespace QTest
{
	template<>
	char* toString (const LeechCraft::Azoth::MuCommands::OpenUrlParams_t& var)
	{
		return PrintVar (var);
	}
}

namespace LeechCraft
{
namespace Azoth
{
namespace MuCommands
{
	namespace
	{
		bool operator== (const UrlRange& r1, const UrlRange& r2)
		{
			return r1.Start_ == r2.Start_ &&
					r1.End_ == r2.End_;
		}

		bool operator== (const UrlComposite& r1, const UrlComposite& r2)
		{
			return r1.Pat_ == r2.Pat_;
		}

		bool operator== (const SinceLast&, const SinceLast&)
		{
			return true;
		}
	}

	void OpenUrlCommandTest::parseLast ()
	{
		const QString command = "/openurl last";
		const auto& res = ParseCommand (command);
		QCOMPARE (res, (OpenUrlParams_t { UrlComposite { SinceLast {}, {} } }));
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
		//QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { {}, {} }, std::string { "^[a-c]$" } } }));
		QCOMPARE (res, (OpenUrlParams_t { RegExpStr_t { "^[a-c]$" } }));
	}

	void OpenUrlCommandTest::parseByRxSpaces ()
	{
		const QString command = "/openurl rx ^[a-c] space smth$";
		const auto& res = ParseCommand (command);
		//QCOMPARE (res, (OpenUrlParams_t { UrlComposite { UrlRange { {}, {} }, std::string { "^[a-c] space smth$" } } }));
		QCOMPARE (res, (OpenUrlParams_t { RegExpStr_t { "^[a-c] space smth$" } }));
	}
}
}
}
