/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bodymarkuptest.h"
#include <QTest>
#include <QXmlStreamReader>
#include <util/sll/qtutil.h>
#include <util/test/randomwalk.h>
#include <util/xpc/util.h>
#include "../bodymarkup.h"

QTEST_MAIN (LC::Sysnotify::BodyMarkupTest)

namespace LC::Sysnotify
{
	void BodyMarkupTest::initMain ()
	{
		qputenv ("QT_QPA_PLATFORM", "offscreen");
	}

	void BodyMarkupTest::testConvertsMarkup_data ()
	{
		QTest::addColumn<QString> ("rich");
		QTest::addColumn<QString> ("body");

		QTest::newRow ("empty") << QString {} << QString {};
		QTest::newRow ("no markup") << "Tom and Jerry"_qs << "Tom and Jerry"_qs;
		QTest::newRow ("entities")
				<< "Tom &amp; Jerry: 2 &lt; 3 &gt; 1, &quot;so&quot;"_qs
				<< "Tom &amp; Jerry: 2 &lt; 3 &gt; 1, &quot;so&quot;"_qs;
		QTest::newRow ("unescaped ampersand") << "R&D"_qs << "R&amp;D"_qs;
		QTest::newRow ("no-break space") << "a&nbsp;b"_qs << u"a\u00a0b"_qs;
		QTest::newRow ("outside the BMP") << u"\U0001F600 smile"_qs << u"\U0001F600 smile"_qs;
		QTest::newRow ("characters XML cannot carry")
				<< "bell"_qs + QChar { 0x7 } + ", lone surrogate"_qs + QChar { 0xd83d }
				<< u"bell, lone surrogate\ufffd"_qs;

		QTest::newRow ("bold, italic, underlined")
				<< "<b>bold</b>, <i>italic</i>, <u>underlined</u>"_qs
				<< "<b>bold</b>, <i>italic</i>, <u>underlined</u>"_qs;
		QTest::newRow ("strong and emphasis")
				<< "<strong>strong</strong>, <em>emphasis</em>"_qs
				<< "<b>strong</b>, <i>emphasis</i>"_qs;
		QTest::newRow ("weight style")
				<< R"(<span style="font-weight: 600">demibold</span>)"_qs
				<< "<b>demibold</b>"_qs;
		QTest::newRow ("decoration style")
				<< R"(<span style="text-decoration: underline">underlined</span>)"_qs
				<< "<u>underlined</u>"_qs;
		QTest::newRow ("nested") << "<b>bold <i>and italic</i></b>"_qs << "<b>bold </b><b><i>and italic</i></b>"_qs;
		QTest::newRow ("other formatting")
				<< R"(<s>struck</s>, <span style="color: red">red</span>)"_qs
				<< "struck, red"_qs;
		QTest::newRow ("image") << R"(an <img src="file:///nowhere.png" alt="alt"/>image)"_qs << "an image"_qs;
		QTest::newRow ("image in bold") << R"(<b><img src="file:///nowhere.png" alt="alt"/></b>text)"_qs << "text"_qs;

		QTest::newRow ("line break") << "one<br/>two"_qs << "one\ntwo"_qs;
		QTest::newRow ("line break in bold") << "<b>one<br/>two</b>"_qs << "<b>one\ntwo</b>"_qs;
		QTest::newRow ("line feed in the source") << "one\ntwo"_qs << "one two"_qs;
		QTest::newRow ("paragraphs") << "<p>first</p><p>second</p>"_qs << "first\nsecond"_qs;
		QTest::newRow ("heading") << "<h3>Heading</h3>text"_qs << "<b>Heading</b>\ntext"_qs;
		QTest::newRow ("list") << "before<ul><li>one</li><li>two</li></ul>after"_qs << "before\none\ntwo\nafter"_qs;
		QTest::newRow ("document")
				<< "<html><body><p>Full <b>document</b></p><p>second paragraph</p></body></html>"_qs
				<< "Full <b>document</b>\nsecond paragraph"_qs;
	}

	void BodyMarkupTest::testConvertsMarkup ()
	{
		QFETCH (QString, rich);
		QFETCH (QString, body);

		QCOMPARE (ToBodyMarkup (Util::RichTextString { rich }, true), body);
		QCOMPARE (ToBodyMarkup (Util::RichTextString { rich }, false), body);
	}

	void BodyMarkupTest::testKeepsLinksOnlyIfAsked_data ()
	{
		QTest::addColumn<QString> ("rich");
		QTest::addColumn<QString> ("withLinks");
		QTest::addColumn<QString> ("withoutLinks");

		QTest::newRow ("link")
				<< R"(see <a href="http://example.org/">the site</a> for more)"_qs
				<< R"(see <a href="http://example.org/">the site</a> for more)"_qs
				<< "see the site for more"_qs;
		QTest::newRow ("target to escape")
				<< R"(<a href="http://example.org/?a=1&amp;b=&quot;2&quot;">link</a>)"_qs
				<< R"(<a href="http://example.org/?a=1&amp;b=&quot;2&quot;">link</a>)"_qs
				<< "link"_qs;
		QTest::newRow ("bold link")
				<< R"(<a href="http://example.org/"><b>bold</b> link</a>)"_qs
				<< R"(<a href="http://example.org/"><b>bold</b></a><a href="http://example.org/"> link</a>)"_qs
				<< "<b>bold</b> link"_qs;
		QTest::newRow ("no target") << "<a>anchor</a>"_qs << "anchor"_qs << "anchor"_qs;
		QTest::newRow ("empty target") << R"(<a href="">anchor</a>)"_qs << "anchor"_qs << "anchor"_qs;
	}

	void BodyMarkupTest::testKeepsLinksOnlyIfAsked ()
	{
		QFETCH (QString, rich);
		QFETCH (QString, withLinks);
		QFETCH (QString, withoutLinks);

		QCOMPARE (ToBodyMarkup (Util::RichTextString { rich }, true), withLinks);
		QCOMPARE (ToBodyMarkup (Util::RichTextString { rich }, false), withoutLinks);
	}

	namespace
	{
		const QChar Bell { 7 };

		const QStringList Texts
		{
			"Tom"_qs, "Jerry"_qs, " and "_qs, " "_qs, "  "_qs, "\n"_qs, "\t"_qs,
			"R&D"_qs, " < "_qs, " > "_qs, "'"_qs, "\""_qs,
			"&amp;"_qs, "&lt;"_qs, "&gt;"_qs, "&quot;"_qs, "&nbsp;"_qs,
			u"\u00e9\u4e2d"_qs, u"\U0001F600"_qs, QString { Bell },
		};

		struct Element
		{
			QString Open_;
			QString Close_;
		};

		const QList<Element> Elements
		{
			{ "<b>"_qs, "</b>"_qs },
			{ "<i>"_qs, "</i>"_qs },
			{ "<u>"_qs, "</u>"_qs },
			{ "<strong>"_qs, "</strong>"_qs },
			{ "<em>"_qs, "</em>"_qs },
			{ "<s>"_qs, "</s>"_qs },
			{ R"(<span style="color: red">)"_qs, "</span>"_qs },
			{ R"(<a href="http://example.org/?a=1&amp;b=&quot;2&quot;">)"_qs, "</a>"_qs },
			{ "<p>"_qs, "</p>"_qs },
			{ "<h3>"_qs, "</h3>"_qs },
			{ "<ul><li>"_qs, "</li></ul>"_qs },
		};

		const QStringList Standalone
		{
			"<br/>"_qs,
			R"(<img src="file:///nowhere.png" alt="alt"/>)"_qs,
			"</b>"_qs,
			"</p>"_qs,
		};

		struct Coverage
		{
			int Formats_ = 0;
			int Links_ = 0;
			int Escapes_ = 0;
			int MultiLine_ = 0;
			int XmlInvalid_ = 0;
		};

		Coverage Seen;

		std::optional<QString> FindXmlProblem (const QString& body, bool withLinks)
		{
			QXmlStreamReader reader { "<body>"_qs + body + "</body>"_qs };
			bool isWrapper = true;
			while (!reader.atEnd ())
			{
				if (reader.readNext () != QXmlStreamReader::StartElement || std::exchange (isWrapper, false))
					continue;

				const auto& name = reader.name ();
				const auto& attrs = reader.attributes ();
				if (name == "a"_ql)
				{
					if (!withLinks)
						return "a link nobody asked for"_qs;
					if (attrs.size () != 1 || attrs.value ("href"_ql).isEmpty ())
						return "a link without exactly a target"_qs;
					++Seen.Links_;
				}
				else if (name == "b"_ql || name == "i"_ql || name == "u"_ql)
				{
					if (!attrs.isEmpty ())
						return "attributes of <%1>"_qs.arg (name);
					++Seen.Formats_;
				}
				else
					return "the element <%1>"_qs.arg (name);
			}
			if (reader.hasError ())
				return reader.errorString ();
			return {};
		}

		QStringList GetShownLines (const Util::RichTextString& text)
		{
			auto lines = Util::ToPlainText (text).remove (Bell).split ('\n');
			for (auto& line : lines)
				line = line.simplified ();
			return lines;
		}

		struct MarkupWalk
		{
			QString Markup_;
			QStringList Unclosed_;

			void AddText (Util::Choices& choices)
			{
				Markup_ += Texts [choices.Draw (Texts.size ())];
			}

			void AddStandalone (Util::Choices& choices)
			{
				Markup_ += Standalone [choices.Draw (Standalone.size ())];
			}

			void Open (Util::Choices& choices)
			{
				const auto& element = Elements [choices.Draw (Elements.size ())];
				Markup_ += element.Open_;
				Unclosed_ << element.Close_;
			}

			void Close (Util::Choices&)
			{
				if (!Unclosed_.isEmpty ())
					Markup_ += Unclosed_.takeLast ();
			}

			std::optional<QString> Check () const
			{
				const Util::RichTextString rich { Markup_ };
				const auto& shown = GetShownLines (rich);
				Seen.MultiLine_ += shown.size () > 1;
				Seen.XmlInvalid_ += Markup_.contains (Bell);

				for (const auto withLinks : { true, false })
				{
					const auto& body = ToBodyMarkup (rich, withLinks);
					Seen.Escapes_ += body.contains ('&');
					const auto report = [&] (const QString& what)
					{
						return "%1 in [%2] made %3 links from [%4]"_qs
								.arg (what, body, withLinks ? "with"_qs : "without"_qs, Markup_);
					};

					if (const auto& problem = FindXmlProblem (body, withLinks))
						return report (*problem);

					const Util::RichTextString readBack { QString { body }.replace ('\n', "<br/>"_ql) };
					if (GetShownLines (readBack) != shown)
						return report ("other lines than the original shows"_qs);
				}
				return {};
			}
		};
	}

	void BodyMarkupTest::testRandomMarkup ()
	{
		Util::RunRandomWalks<MarkupWalk> ({ .Seed_ = 20261126, .Walks_ = 300, .Steps_ = 40 },
				{
					{ "text"_qs, &MarkupWalk::AddText },
					{ "standalone"_qs, &MarkupWalk::AddStandalone },
					{ "open"_qs, &MarkupWalk::Open },
					{ "close"_qs, &MarkupWalk::Close },
				});
		if (QTest::currentTestFailed ())
			return;

		QCOMPARE_GT (Seen.Formats_, 0);
		QCOMPARE_GT (Seen.Links_, 0);
		QCOMPARE_GT (Seen.Escapes_, 0);
		QCOMPARE_GT (Seen.MultiLine_, 0);
		QCOMPARE_GT (Seen.XmlInvalid_, 0);
	}
}
