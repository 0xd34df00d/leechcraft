/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "parse_test.h"
#include <QDomDocument>
#include <QUrl>
#include <QtTest>
#include <components/parsers/parse.h>
#include "item.h"

namespace LC::Aggregator::Parsers
{
	namespace
	{
		QDomDocument ParseXml (const QByteArray& xml)
		{
			QDomDocument doc;
			const auto result = doc.setContent (xml, QDomDocument::ParseOption::UseNamespaceProcessing);
			if (!result)
				qFatal () << "cannot parse test document:" << result.errorMessage;
			return doc;
		}

		const QUrl FeedUrl { "http://example.com/feeds/atom.xml" };
	}

	void ParseTest::testFeedRootNames ()
	{
		QVERIFY (IsFeedRootName (u"rss"));
		QVERIFY (IsFeedRootName (u"feed"));
		QVERIFY (IsFeedRootName (u"RDF"));

		QVERIFY (!IsFeedRootName (u"atom"));
		QVERIFY (!IsFeedRootName (u"html"));
		QVERIFY (!IsFeedRootName (u"opml"));
	}

	void ParseTest::testRssVersions ()
	{
		for (const auto& version : { "", " version=\"0.91\"", " version=\"0.94\"", " version=\"2.0\"", " version=\"2.0.1\"" })
		{
			const auto& doc = ParseXml (QByteArray { "<rss" } + version + "><channel><title>T</title><link>http://example.com/</link>"
					"<item><title>I</title><link>http://example.com/1</link></item></channel></rss>");
			const auto& channels = TryParse (doc, 1, FeedUrl);
			QVERIFY2 (channels, version);
			QCOMPARE (channels->size (), 1uz);
			QCOMPARE (channels->at (0)->Title_, "T");
			QCOMPARE (channels->at (0)->Items_.size (), qsizetype { 1 });
		}
	}

	void ParseTest::testRelativeUrls ()
	{
		const auto& withBase = ParseXml (R"(<feed xmlns="http://www.w3.org/2005/Atom" xml:base="http://example.com/blog/">
				<title>T</title><link href="./"/>
				<entry><title>I</title><link href="post/1"/><link rel="enclosure" href="../media/1.mp3" type="audio/mpeg"/>
					<content type="xhtml"><div xmlns="http://www.w3.org/1999/xhtml">text</div></content></entry>
			</feed>)");
		const auto& channels = TryParse (withBase, 1, FeedUrl);
		QVERIFY (channels);
		const auto& channel = *channels->at (0);
		QCOMPARE (channel.Link_, "http://example.com/blog/");
		const auto& item = *channel.Items_.at (0);
		QCOMPARE (item.Link_, "http://example.com/blog/post/1");
		QCOMPARE (item.Enclosures_.size (), qsizetype { 1 });
		QCOMPARE (item.Enclosures_.at (0).URL_, "http://example.com/media/1.mp3");

		const auto& noBase = ParseXml (R"(<rss version="2.0"><channel><title>T</title><link>/</link>
				<image><url>img/logo.png</url></image>
				<item><title>I</title><link>post/1</link><enclosure url="//cdn.example.org/1.mp3" type="audio/mpeg" length="1"/></item>
				<item><title>J</title><link>https://other.example.org/2</link></item>
			</channel></rss>)");
		const auto& rssChannels = TryParse (noBase, 1, FeedUrl);
		QVERIFY (rssChannels);
		const auto& rssChannel = *rssChannels->at (0);
		QCOMPARE (rssChannel.Link_, "http://example.com/");
		QCOMPARE (rssChannel.PixmapURL_, "http://example.com/feeds/img/logo.png");
		QCOMPARE (rssChannel.Items_.at (0)->Link_, "http://example.com/feeds/post/1");
		QCOMPARE (rssChannel.Items_.at (0)->Enclosures_.at (0).URL_, "http://cdn.example.org/1.mp3");
		QCOMPARE (rssChannel.Items_.at (1)->Link_, "https://other.example.org/2");
	}
}
