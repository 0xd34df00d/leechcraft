/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urldecodertest.h"
#include <QtTest>
#include "../urldecoder.cpp"

QTEST_APPLESS_MAIN (LC::Azoth::Acetamide::UrlDecoderTest)

namespace LC::Azoth::Acetamide
{
	auto Decode (const char *str)
	{
		return DecodeUrl (QUrl::fromUserInput (str));
	}

	void UrlDecoderTest::testDefaultServer ()
	{
		for (const auto str : { "irc:", "irc://", "irc:///" })
		{
			auto res = Decode (str);
			QVERIFY2 (static_cast<bool> (res), "parsing succeeded");
			QCOMPARE (res->Server_.ServerName_, QString {});
			QCOMPARE (res->Target_, Target { NoTarget {} });
		}

		auto res = Decode ("irc:///help");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");
		QCOMPARE (res->Server_.ServerName_, QString {});
		QCOMPARE (res->Target_, Target { ChannelTarget { .Opts_ = ChannelOptions { .ChannelName_ = "help" } } });
	}

	void UrlDecoderTest::testServerChannel ()
	{
		Target refTarget = ChannelTarget
		{
			.Opts_ = ChannelOptions { .ServerName_ = "foobar.org", .ChannelName_ = "secret" },
			.HasPassword_ = true,
		};

		auto res = Decode ("irc://foobar.org:6665/secret,needkey");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerPort_, 6665);
		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->HasServerPassword_, false);
		QCOMPARE (res->Target_, refTarget);

		res = Decode ("irc://foobar.org:6665/secret,needkey,needpass");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerPort_, 6665);
		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->HasServerPassword_, true);
		QCOMPARE (res->Target_, refTarget);

		refTarget = ChannelTarget
		{
			.Opts_ = ChannelOptions { .ServerName_ = "foobar.org", .ChannelName_ = "#secret" },
			.HasPassword_ = false,
		};
		res = Decode ("irc://foobar.org:6665/#secret,needpass");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerPort_, 6665);
		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->HasServerPassword_, true);
		QCOMPARE (res->Target_, refTarget);
	}

	void UrlDecoderTest::testNickOnly ()
	{
		auto res = Decode ("irc://foobar.org/Mmmm,isnick,needpass");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->HasServerPassword_, true);

		const auto nickOnly = std::get_if<NickOnly> (&res->Target_);
		QVERIFY2 (nickOnly, "is indeed a NickOnly");
		QCOMPARE (nickOnly->Nick_, "Mmmm");
	}

	void UrlDecoderTest::testNickInfo ()
	{
		auto res = Decode ("irc://foobar.org/Mmmm!mandar@*uoknor.edu,isnick,needpass");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->HasServerPassword_, true);

		const auto nickInfo = std::get_if<NickInfo> (&res->Target_);
		QVERIFY2 (nickInfo, "is indeed a NickInfo");
		QCOMPARE (nickInfo->Nick_, "Mmmm");
		QCOMPARE (nickInfo->User_, "mandar");
		QCOMPARE (nickInfo->HostMask_, "*uoknor.edu");
	}

	void UrlDecoderTest::testUserInfo ()
	{
		auto res = Decode ("irc://foobar.org/mandar@uoknor.edu,isnick,needpass");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->HasServerPassword_, true);

		const auto userInfo = std::get_if<UserInfo> (&res->Target_);
		QVERIFY2 (userInfo, "is indeed a UserInfo");
		QCOMPARE (userInfo->User_, "mandar");
		QCOMPARE (userInfo->ServerName_, "uoknor.edu");
	}
}
