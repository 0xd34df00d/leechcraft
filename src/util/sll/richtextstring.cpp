/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "richtextstring.h"
#include "qtutil.h"

namespace LC::Util
{
	namespace
	{
		constexpr char16_t LineTabulation = u'\v';
		constexpr char16_t NextLine = u'\u0085';

		bool IsLineBreak (QChar ch)
		{
			switch (ch.unicode ())
			{
			case QChar::LineFeed:
			case LineTabulation:
			case QChar::FormFeed:
			case QChar::CarriageReturn:
			case NextLine:
			case QChar::LineSeparator:
			case QChar::ParagraphSeparator:
				return true;
			default:
				return false;
			}
		}

		QString ToMarkup (const QString& plain)
		{
			const auto& escaped = plain.toHtmlEscaped ();

			QString markup;
			markup.reserve (escaped.size ());
			for (auto pos = escaped.cbegin (); pos != escaped.cend (); ++pos)
			{
				if (!IsLineBreak (*pos))
				{
					markup += *pos;
					continue;
				}

				markup += "<br/>"_ql;

				const auto next = pos + 1;
				if (*pos == QChar::CarriageReturn && next != escaped.cend () && *next == QChar::LineFeed)
					pos = next;
			}
			return markup;
		}
	}

	RichTextString::RichTextString (const PlainTextString& plain)
	: Html_ { ToMarkup (plain.Text_) }
	{
	}
}
