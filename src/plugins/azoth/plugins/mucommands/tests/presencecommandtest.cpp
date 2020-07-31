/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "presencecommandtest.h"
#include <QtTest>
#include <QtDebug>
#include "presencecommand.cpp"
#include "testutil.h"

QTEST_APPLESS_MAIN (LC::Azoth::MuCommands::PresenceCommandTest)

namespace QTest
{
	template<>
	char* toString (const LC::Azoth::MuCommands::Status_t& acc)
	{
		return LC::Azoth::MuCommands::PrintVar (acc);
	}
}

namespace LC
{
namespace Azoth
{
namespace MuCommands
{
	bool operator== (const AllAccounts&, const AllAccounts&) { return true; }
	bool operator== (const CurrentAccount&, const CurrentAccount&) { return true; }
	bool operator== (const ClearStatus&, const ClearStatus&) { return true; }

	void PresenceCommandTest::accStateChange ()
	{
		const QString command = R"delim(
/presence testacc with spaces
away
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc with spaces" } });
		QCOMPARE (res.Status_, Status_t { State_t { State::SAway } });
	}

	void PresenceCommandTest::allAccStateChange()
	{
		const QString command = R"delim(
/presence *
away
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { AllAccounts {} });
		QCOMPARE (res.Status_, Status_t { State_t { State::SAway } });
	}

	void PresenceCommandTest::noAccStateChange()
	{
		const QString command = R"delim(
/presence
away
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { CurrentAccount {} });
		QCOMPARE (res.Status_, Status_t { State_t { State::SAway } });
	}

	void PresenceCommandTest::accAlmostCustomStateChange ()
	{
		const QString command = R"delim(
/presence testacc with spaces
some custom status
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc with spaces" } });
		QCOMPARE (res.Status_, Status_t { std::string { "some custom status" } });
	}

	void PresenceCommandTest::accCustomStateChange ()
	{
		const QString command = R"delim(
/presence testacc with spaces
some custom status
				)delim";
		const auto& res = ParsePresenceCommand (command,
				{ "some custom status", "another custom status", "some custom status too" });
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc with spaces" } });
		QCOMPARE (res.Status_, Status_t { State_t { "some custom status" } });
	}

	void PresenceCommandTest::accStatusChange ()
	{
		const QString command = R"delim(
/presence testacc
xa
This is my new
multiline status.
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc" } });

		const auto expectedStatus = Status_t { FullState_t { State::SXA, "This is my new\nmultiline status." } };
		QCOMPARE (res.Status_, expectedStatus);
	}

	void PresenceCommandTest::accStatusChangeSameLine ()
	{
		const QString command = R"delim(
/presence testacc
away oh yeah i'm going away
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc" } });

		const auto expectedStatus = Status_t { FullState_t { State::SAway, "oh yeah i'm going away" } };
		QCOMPARE (res.Status_, expectedStatus);
	}

	void PresenceCommandTest::accStatusClear ()
	{
		const QString command = R"delim(
/presence testacc
clear
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc" } });
		QCOMPARE (res.Status_, Status_t { ClearStatus {} });
	}

	void PresenceCommandTest::accStatusChangeClearSubstr ()
	{
		const QString command = R"delim(
/presence testacc
clearr
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc" } });
		QCOMPARE (res.Status_, Status_t { std::string { "clearr" } });
	}

	void PresenceCommandTest::accMessageOnly ()
	{
		const QString command = R"delim(
/presence testacc with spaces

away
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc with spaces" } });
		QCOMPARE (res.Status_, Status_t { std::string { "away" } });
	}

	void PresenceCommandTest::accMultilineMessageOnly ()
	{

		const QString command = R"delim(
/presence testacc
This is my new
multiline status.
				)delim";
		const auto& res = ParsePresenceCommand (command);
		QCOMPARE (res.AccName_, AccName_t { std::string { "testacc" } });

		const auto expectedStatus = Status_t { std::string { "This is my new\nmultiline status." } };
		QCOMPARE (res.Status_, expectedStatus);
	}

	void PresenceCommandTest::chatPrStateChangeNoNL ()
	{
		const QString command = R"delim(
/chatpresence away
				)delim";
		const auto& res = ParseChatPresenceCommand (command);
		QCOMPARE (res.Status_, Status_t { State_t { State::SAway } });
	}

	void PresenceCommandTest::chatPrAlmostCustomStateChangeNoNL ()
	{
		const QString command = R"delim(
/chatpresence some custom status
				)delim";
		const auto& res = ParseChatPresenceCommand (command);
		QCOMPARE (res.Status_, Status_t { std::string { "some custom status" } });
	}

	void PresenceCommandTest::chatPrCustomStateChangeNoNL ()
	{
		const QString command = R"delim(
/chatpresence some custom status
				)delim";
		const auto& res = ParseChatPresenceCommand (command,
				{ "some custom status", "another custom status", "some custom status too" });
		QCOMPARE (res.Status_, Status_t { State_t { "some custom status" } });
	}

	void PresenceCommandTest::chatPrStatusChangeNoNL ()
	{
		const QString command = R"delim(
/chatpresence xa
This is my new
multiline status.
				)delim";
		const auto& res = ParseChatPresenceCommand (command);
		const auto expectedStatus = Status_t { FullState_t { State::SXA, "This is my new\nmultiline status." } };
		QCOMPARE (res.Status_, expectedStatus);
	}

	void PresenceCommandTest::chatPrStatusChangeSameLineNoNL ()
	{
		const QString command = R"delim(
/chatpresence away oh yeah i'm going away
				)delim";
		const auto& res = ParseChatPresenceCommand (command);
		const auto expectedStatus = Status_t { FullState_t { State::SAway, "oh yeah i'm going away" } };
		QCOMPARE (res.Status_, expectedStatus);
	}
}
}
}
