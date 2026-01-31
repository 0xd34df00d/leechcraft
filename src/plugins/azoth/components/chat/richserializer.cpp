/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "richserializer.h"
#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>
#include <util/sll/qtutil.h>

namespace LC::Azoth
{
	RichSerializer::RichSerializer (const QTextDocument& doc)
	: StockFamilies_ { QTextDocument {}.begin ().charFormat ().fontFamilies ().toStringList () }
	{
		QStringList substrs;
		for (auto block = doc.begin (); block != doc.end (); block = block.next ())
		{
			const auto [blockStart, blockEnd] = GetBlockTags (block.blockFormat ());
			substrs << blockStart;

			for (auto it = block.begin (); !it.atEnd (); ++it)
			{
				if (it.fragment ().text ().isEmpty ())
					continue;

				const auto [fragStart, fragEnd] = GetFragmentTags (it.fragment ().charFormat ());
				substrs += fragStart;
				substrs << it.fragment ().text ().toHtmlEscaped ();
				substrs += fragEnd;
			}

			substrs << blockEnd;
		}

		if (substrs.endsWith ("<br/>"_qs))
			substrs.pop_back ();

		Xhtml_ = substrs.join (QString {});
	}

	bool RichSerializer::HasCustomFormatting () const
	{
		return HasCustomFormatting_;
	}

	QString RichSerializer::GetXhtml () const
	{
		return Xhtml_;
	}

	QPair<QString, QString> RichSerializer::GetBlockTags (const QTextBlockFormat& fmt)
	{
		if (fmt.hasProperty (QTextBlockFormat::Property::BlockAlignment))
		{
			HasCustomFormatting_ = true;

			auto align = "left"_qs;
			switch (fmt.alignment ())
			{
			case Qt::AlignCenter:
				align = "center"_qs;
				break;
			case Qt::AlignRight:
				align = "right"_qs;
				break;
			case Qt::AlignJustify:
				align = "justify"_qs;
				break;
			default:
				break;
			}
			return { R"(<p style="text-align:%1">)"_qs.arg (align), "</p>"_qs };
		}

		return { {}, "<br/>"_qs };
	}

	QStringList RichSerializer::GetSpanStyle (const QTextCharFormat& fmt) const
	{
		QStringList attrs;

		const auto isUnderline = fmt.fontUnderline ();
		const auto isStrikeThrough = fmt.fontStrikeOut ();
		if (isUnderline || isStrikeThrough)
		{
			QStringList values;
			if (isUnderline)
				values << "underline"_qs;
			if (isStrikeThrough)
				values << "line-through"_qs;
			attrs << "text-decoration:" + values.join (' ') + ';';
		}

		if (fmt.hasProperty (QTextCharFormat::ForegroundBrush))
		{
			const auto& color = fmt.foreground ().color ();
			attrs << "color:"_qs + color.name () + ';';
		}

		if (fmt.hasProperty (QTextCharFormat::Property::FontPointSize))
			attrs << "font-size:%1pt;"_qs.arg (fmt.fontPointSize ());

		if (fmt.hasProperty (QTextCharFormat::FontFamilies))
			if (const auto& families = fmt.fontFamilies ().toStringList ();
				families != StockFamilies_)
			{
				const auto& family = families.value (0).toHtmlEscaped ();
				attrs << R"(font-family:"%1";)"_qs.arg (family);
			}

		return attrs;
	}

	QPair<QStringList, QStringList> RichSerializer::GetFragmentTags (const QTextCharFormat& fmt)
	{
		QStringList open;
		QStringList close;

		if (fmt.fontWeight () >= QFont::Bold)
		{
			open << "<strong>"_qs;
			close << "</strong>"_qs;
		}
		if (fmt.fontItalic ())
		{
			open << "<em>"_qs;
			close << "</em>"_qs;
		}
		if (const auto& spanStyle = GetSpanStyle (fmt);
			!spanStyle.isEmpty ())
		{
			open << "<span style='"_qs + spanStyle.join (' ') + "'>"_qs;
			close << "</span>"_qs;
		}

		if (!open.isEmpty ())
			HasCustomFormatting_ = true;

		std::ranges::reverse (close);
		return { open, close };
	}
}
