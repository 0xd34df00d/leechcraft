/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "writepdf.h"
#include <QPrinter>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>

namespace LC::Aggregator
{
	namespace
	{
		void WritePDFChannel (QTextCursor& cursor,
				const ChannelShort& cs, const QList<Item>& items,
				const QTextFrame *topFrame, int baseFontSize, const QFont& font)
		{
			auto origCharFmt = cursor.charFormat ();
			origCharFmt.setFontPointSize (baseFontSize);

			QTextFrameFormat frameFmt;
			frameFmt.setBorder (1);
			frameFmt.setPadding (5);
			frameFmt.setBackground (QColor ("#A4C0E4"));
			cursor.insertFrame (frameFmt);

			auto titleFmt = origCharFmt;
			titleFmt.setFontWeight (QFont::Bold);
			titleFmt.setFontPointSize (baseFontSize * 1.35);
			cursor.setCharFormat (titleFmt);
			cursor.insertText (cs.Title_);

			cursor.setPosition (topFrame->lastPosition ());

			titleFmt.setFontWeight (QFont::DemiBold);
			titleFmt.setFontPointSize (baseFontSize * 1.20);
			auto dateFmt = origCharFmt;
			dateFmt.setFontItalic (true);
			for (const auto& item : items)
			{
				cursor.setCharFormat (titleFmt);
				cursor.insertText (item.Title_ + "\n");

				cursor.setCharFormat (dateFmt);
				cursor.insertText (item.PubDate_.toString ());

				cursor.setCharFormat (origCharFmt);

				auto descr = item.Description_;
				descr.remove (QRegularExpression { "<img .*?>" });
				descr.remove ("</img>");

				descr.remove (QRegularExpression { "<a.*?></a>" });
				descr += "<br/><br/>";

				descr.prepend (QString ("<div style='font-size: %1pt; font-family: %2;'>")
							.arg (baseFontSize)
							.arg (font.family ()));
				descr.append ("</div>");

				cursor.insertHtml (descr);
			}
		}
	}

	void WritePDF (const PdfConfig& config, const QMap<ChannelShort, QList<Item>>& channels)
	{
		QTextDocument doc;
		QTextCursor cursor (&doc);

		auto topFrame = cursor.currentFrame ();
		QTextFrameFormat frameFmt;
		frameFmt.setBorder (1);
		frameFmt.setPadding (10);
		frameFmt.setBackground (QColor ("#A4C0E4"));
		cursor.insertFrame(frameFmt);

		auto origFmt = cursor.charFormat ();
		origFmt.setFont (config.Font_);
		origFmt.setFontPointSize (config.FontSize_);

		auto titleFmt = origFmt;
		titleFmt.setFontWeight (QFont::Bold);
		titleFmt.setFontPointSize (config.FontSize_ * 1.5);
		cursor.setCharFormat (titleFmt);
		cursor.insertText (config.Title_);

		cursor.setPosition (topFrame->lastPosition ());

		cursor.setCharFormat (origFmt);
		for (const auto& [channel, items] : channels.asKeyValueRange ())
			WritePDFChannel (cursor, channel, items, topFrame, config.FontSize_, config.Font_);

		QPrinter printer;
		printer.setOutputFileName (config.Filename_);
		printer.setOutputFormat (QPrinter::PdfFormat);
		printer.setFontEmbeddingEnabled (true);
		printer.setPageMargins (config.Margins_, QPageLayout::Millimeter);
		printer.setPageSize (config.PageSize_);

		doc.print (&printer);
	}
}
