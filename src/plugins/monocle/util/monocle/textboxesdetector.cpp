/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textboxesdetector.h"
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <interfaces/monocle/ihavetextcontent.h>

namespace LC::Monocle
{
	namespace
	{
		template<typename It>
		auto GetNextSymbol (It nextWordStart, It lineEnd, It blockEnd)
		{
			if (nextWordStart == lineEnd)
				return NextSpaceKind::NewLine;
			if (nextWordStart == blockEnd)
				return NextSpaceKind::NewPara;

			return NextSpaceKind::Space;
		}

		QVector<TextBox> HandleBlock (const QTextBlock& block, const QRectF& blockRect, const QRectF& pageRect)
		{
			QVector<TextBox> result;
			result.reserve (block.text ().count (' '));

			const auto& text = block.text ();

			const auto layout = block.layout ();
			for (int i = 0, lc = layout->lineCount (); i < lc; ++i)
			{
				const auto line = layout->lineAt (i);
				const auto& lineRectInDoc = line.rect ().translated (blockRect.topLeft ());
				if (!lineRectInDoc.intersects (pageRect))
					continue;

				const auto lineXPos = lineRectInDoc.left () - (line.lineNumber () ? 0 : block.blockFormat ().textIndent ());

				const auto lineEnd = text.begin () + line.textStart () + line.textLength ();
				auto nextWordStart = text.begin () + line.textStart ();
				while (nextWordStart != lineEnd)
				{
					const auto wordStart = nextWordStart;
					const auto wordEnd = std::find_if (nextWordStart + 1, lineEnd, [] (QChar ch) { return ch.isSpace (); });

					const auto textStartPos = static_cast<int> (wordStart - text.begin ());
					const auto len = static_cast<int> (wordEnd - wordStart);

					PageAbsoluteRectBase wordRect { lineRectInDoc.translated (-pageRect.topLeft ()) };
					wordRect.SetLeft (line.cursorToX (textStartPos) + lineXPos);
					wordRect.SetRight (line.cursorToX (textStartPos + len) + lineXPos);

					nextWordStart = std::find_if (wordEnd, lineEnd, [] (QChar ch) { return !ch.isSpace (); });

					result.push_back ({
							.Text_ = text.mid (textStartPos, len),
							.Rect_ = wordRect.ToPageRelative (pageRect.size ()),
							.NextSpaceKind_ = GetNextSymbol (nextWordStart, lineEnd, text.end ()),
						});
				}
			}

			return result;
		}
	}

	QVector<TextBox> DetectTextBoxes (QTextDocument& doc, int page)
	{
		const auto& pageSize = doc.pageSize ();
		const QRectF pageRect { QPointF { 0, pageSize.height () * page }, pageSize };

		const auto docLayout = doc.documentLayout ();

		QVector<TextBox> result;
		for (auto block = doc.begin (); block != doc.end (); block = block.next ())
		{
			const auto& blockRect = docLayout->blockBoundingRect (block);
			if (blockRect.intersects (pageRect))
				result += HandleBlock (block, blockRect, pageRect);
		}
		return result;
	}
}
