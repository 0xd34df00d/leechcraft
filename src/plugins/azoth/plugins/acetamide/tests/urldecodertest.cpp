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
			QVERIFY (res->Server_.ServerName_.isEmpty ());
			QVERIFY (res->Channel_.ChannelName_.isEmpty ());
		}

		auto res = Decode ("irc:///help");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");
		QVERIFY (res->Server_.ServerName_.isEmpty ());
		QCOMPARE (res->Channel_.ChannelName_, "help");
	}

	void UrlDecoderTest::testServerChannel ()
	{
		auto res = Decode ("irc://foobar.org:6665/secret,needkey");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerPort_, 6665);
		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->Channel_.ChannelName_, "secret");
		QCOMPARE (res->Channel_.ServerName_, res->Server_.ServerName_);
		QCOMPARE (res->HasServerPassword_, false);
		QCOMPARE (res->HasChannelPassword_, true);

		res = Decode ("irc://foobar.org:6665/secret,needkey,needpass");
		QVERIFY2 (static_cast<bool> (res), "parsing succeeded");

		QCOMPARE (res->Server_.ServerPort_, 6665);
		QCOMPARE (res->Server_.ServerName_, "foobar.org");
		QCOMPARE (res->Channel_.ChannelName_, "secret");
		QCOMPARE (res->Channel_.ServerName_, res->Server_.ServerName_);
		QCOMPARE (res->HasServerPassword_, true);
		QCOMPARE (res->HasChannelPassword_, true);
	}
}
