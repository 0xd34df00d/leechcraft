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
		const Target refTarget = ChannelTarget
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
	}
}
