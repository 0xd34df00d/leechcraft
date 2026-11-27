/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bodymarkup.h"
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <util/sll/qtutil.h>

namespace LC::Sysnotify
{
	namespace
	{
		bool IsXmlChar (char32_t ch)
		{
			return ch == '\t' || ch == '\n' || ch == '\r' ||
					(ch >= 0x20 && ch <= 0xD7FF) ||
					(ch >= 0xE000 && ch <= 0xFFFD) ||
					(ch >= 0x10000 && ch <= 0x10FFFF);
		}

		QString Escape (const QString& text)
		{
			QString result;
			result.reserve (text.size () * 2);
			for (const char32_t ch : text.toUcs4 ())
				switch (ch)
				{
				case QChar::LineSeparator:
					result += u'\n';
					break;
				case QChar::ObjectReplacementCharacter:
					break;
				case '&':
					result += "&amp;"_ql;
					break;
				case '<':
					result += "&lt;"_ql;
					break;
				case '>':
					result += "&gt;"_ql;
					break;
				case '"':
					result += "&quot;"_ql;
					break;
				default:
					if (IsXmlChar (ch))
						result += QChar::fromUcs4 (ch);
					break;
				}
			return result;
		}

		QString ToMarkup (const QTextFragment& fragment, bool withLinks)
		{
			auto markup = Escape (fragment.text ());
			if (markup.isEmpty ())
				return markup;

			const auto wrap = [&markup] (QLatin1String tag) { markup = "<%1>%2</%1>"_qs.arg (tag, markup); };
			const auto& format = fragment.charFormat ();
			if (format.fontUnderline () && !format.isAnchor ())
				wrap ("u"_ql);
			if (format.fontItalic ())
				wrap ("i"_ql);
			if (format.fontWeight () > QFont::Medium)
				wrap ("b"_ql);
			if (const auto& href = format.anchorHref ();
				withLinks && !href.isEmpty ())
				markup = R"(<a href="%1">%2</a>)"_qs.arg (Escape (href), markup);
			return markup;
		}
	}

	QString ToBodyMarkup (const Util::RichTextString& text, bool withLinks)
	{
		QTextDocument doc;
		doc.setHtml (text.ToHtml ());

		QStringList lines;
		for (auto block = doc.begin (); block.isValid (); block = block.next ())
		{
			QString line;
			for (auto it = block.begin (); !it.atEnd (); ++it)
				line += ToMarkup (it.fragment (), withLinks);
			lines << line;
		}
		return lines.join (u'\n');
	}
}
