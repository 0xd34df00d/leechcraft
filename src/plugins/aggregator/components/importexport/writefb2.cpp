/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "writefb2.h"
#include <ranges>
#include <QFile>
#include <QRegularExpression>
#include <QUuid>
#include <QXmlStreamWriter>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>

namespace LC::Aggregator
{
	namespace
	{
		void WriteDescription (QXmlStreamWriter& w, const QStringList& authors, const QString& name)
		{
			w.writeStartElement ("description");
				w.writeStartElement ("title-info");
					for (const auto& author : authors)
					{
						w.writeStartElement ("author");
							w.writeTextElement ("nickname", author);
						w.writeEndElement ();
					}
					w.writeTextElement ("book-title", name);
					w.writeTextElement ("lang", "en");
				w.writeEndElement ();

				w.writeStartElement ("document-info");
					w.writeStartElement ("author");
						w.writeTextElement ("nickname", "LeechCraft");
					w.writeEndElement ();
					w.writeTextElement ("program-used",
							"LeechCraft Aggregator %1"_qs.arg (GetProxyHolder ()->GetVersion ()));
					w.writeTextElement ("id", QUuid::createUuid ().toString ());
					w.writeTextElement ("version", "1.0");
					w.writeStartElement ("date");
						const auto& date = QDate::currentDate ();
						w.writeAttribute ("date", date.toString (Qt::ISODate));
						w.writeCharacters (date.toString (Qt::TextDate));
					w.writeEndElement ();
				w.writeEndElement ();
			w.writeEndElement ();
		}

		QString FixContents (QString descr)
		{
			descr.replace (QRegularExpression { "</p>\\s*<p>" }, "<br/>");
			descr.remove ("<p>");
			descr.remove ("</p>");

			// Remove images, links and frames
			descr.remove (QRegularExpression { "<img .*?>" });
			descr.remove ("</img>");

			// Remove tables
			if (descr.contains ("<table", Qt::CaseInsensitive))
				descr.remove (QRegularExpression { "<table.*?/table>", QRegularExpression::CaseInsensitiveOption });

			// Objects
			if (descr.contains ("<object", Qt::CaseInsensitive))
				descr.remove (QRegularExpression { "<object.*?/object>", QRegularExpression::CaseInsensitiveOption });

			descr.remove (QRegularExpression { "<a.*?>" });
			descr.remove ("</a>");

			descr.remove (QRegularExpression { "<iframe .*?/iframe>" });

			// Replace HTML entities with corresponding stuff
			descr.replace ("&qout;", "\"");
			descr.replace ("&emdash;", QString::fromUtf8 ("—"));
			descr.replace ("&mdash;", QString::fromUtf8 ("—"));
			descr.replace ("&ndash;", "-");

			// Remove the rest
			descr.replace ("&amp;", "&&");
			descr.remove (QRegularExpression ("&\\w*;"));
			descr.replace ("&&", "&amp;");

			// Fix some common errors
			descr.replace ("<br>", "<br/>");
			descr.replace (QRegularExpression ("<br\\s+/>"), "<br/>");

			// Replace multilines
			while (descr.contains ("<br/><br/>"))
				descr.replace ("<br/><br/>", "<br/>");

			// Replace HTML tags with their fb2 analogues
			descr.replace ("<em>", "<emphasis>", Qt::CaseInsensitive);
			descr.replace ("</em>", "</emphasis>", Qt::CaseInsensitive);
			descr.replace ("<i>", "<emphasis>", Qt::CaseInsensitive);
			descr.replace ("</i>", "</emphasis>", Qt::CaseInsensitive);
			descr.replace ("<b>", "<strong>", Qt::CaseInsensitive);
			descr.replace ("</b>", "</strong>", Qt::CaseInsensitive);
			descr.replace ("<ss>", "<strikethrough>", Qt::CaseInsensitive);
			descr.replace ("</ss>", "</strikethrough>", Qt::CaseInsensitive);

			if (descr.endsWith ("<br/>"))
				descr.chop (5);

			// Remove unclosed tags
			QRegularExpression unclosedRx { "<(\\w+?)[^/]*?>" };
			int pos = 0;
			for (auto match = unclosedRx.match (descr, pos); match.hasMatch (); match = unclosedRx.match (descr, pos))
			{
				auto closeTag = "</" + match.captured (1) + ">";
				if (descr.contains (closeTag))
				{
					pos += match.capturedLength ();
					continue;
				}
				descr.remove (pos, match.capturedLength ());
			}

			// Normalize empty lines - needs to be done after removing
			// unclosed, otherwise last <p> would get dropped.
			descr.replace (QRegularExpression ("<br/>\\s*</p>"), "</p>");
			descr.replace ("<br/>", "</p><p>");

			descr.remove ("\r");
			descr.remove ("\n");
			descr = descr.simplified ();

			return descr;
		}

		void WriteChannel (QXmlStreamWriter& w, const ChannelShort& cs, const QList<Item>& items)
		{
			w.writeStartElement ("section");
				w.writeAttribute ("id", QString::number (cs.ChannelID_));
				w.writeStartElement ("title");
					w.writeTextElement ("p", FixContents (cs.Title_));
				w.writeEndElement ();
				w.writeTextElement ("annotation", QObject::tr ("%n unread item(s)", "", cs.Unread_));
				for (const auto& item : items)
				{
					w.writeStartElement ("title");
						w.writeStartElement ("p");
						w.writeComment ("p");
						w.device ()->write (FixContents (item.Title_).toUtf8 ());
						w.writeEndElement ();
					w.writeEndElement ();

					const auto hasDate = item.PubDate_.isValid ();
					const auto hasAuthor = !item.Author_.isEmpty ();
					if (hasDate || hasAuthor)
					{
						w.writeStartElement ("epigraph");
							if (hasDate)
								w.writeTextElement ("p",
										QObject::tr ("Published on %1").arg (item.PubDate_.toString ()));
							if (hasAuthor)
								w.writeTextElement ("p",
										QObject::tr ("By %1").arg (item.Author_));
						w.writeEndElement ();
						w.writeEmptyElement ("empty-line");
					}

					w.writeStartElement ("p");
						w.writeComment ("p");
						w.device ()->write (FixContents (item.Description_).toUtf8 ());
					w.writeEndElement ();

					w.writeEmptyElement ("empty-line");
				}
			w.writeEndElement ();
		}
	}

	void WriteFB2 (const Fb2Config& config, const QMap<ChannelShort, QList<Item>>& channels, QIODevice& output)
	{
		auto authors = std::ranges::subrange (channels.keyBegin (), channels.keyEnd ())
				| std::views::transform (&ChannelShort::Author_)
				| std::ranges::to<QList> ();
		if (authors.isEmpty ())
			authors << "LeechCraft"_qs;

		QXmlStreamWriter w (&output);
		w.setAutoFormatting (true);
		w.setAutoFormattingIndent (2);
		w.writeStartDocument ();
		w.writeStartElement ("FictionBook");
		w.writeDefaultNamespace ("http://www.gribuser.ru/xml/fictionbook/2.0");
		w.writeNamespace ("http://www.w3.org/1999/xlink", "l");
		WriteDescription (w, authors, config.Title_);

		w.writeStartElement ("body");
		for (const auto& [cs, items] : channels.asKeyValueRange ())
			WriteChannel (w, cs, items);
		w.writeEndElement ();
		w.writeEndElement ();
		w.writeEndDocument ();
	}
}
