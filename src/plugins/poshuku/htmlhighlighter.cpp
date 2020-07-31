/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "htmlhighlighter.h"
#include <QTextEdit>

namespace LC
{
namespace Poshuku
{
	HtmlHighlighter::HtmlHighlighter (QTextDocument *doc)
	: QSyntaxHighlighter (doc)
	{
		Fill ();
	}

	HtmlHighlighter::HtmlHighlighter (QTextEdit *edit)
	: QSyntaxHighlighter (edit->document ())
	{
		Fill ();
	}

	void HtmlHighlighter::SetFormatFor (Construct c, const QTextCharFormat& f)
	{
		Formats_ [c] = f;
		rehighlight ();
	}

	QTextCharFormat HtmlHighlighter::GetFormatFor (Construct c) const
	{
		return Formats_ [c];
	}

	void HtmlHighlighter::highlightBlock (const QString& text)
	{
		int state = previousBlockState ();
		int len = text.length ();
		int start = 0, pos = 0;

		while (pos < len)
		{
			switch (state)
			{
				case InComment:
					start = pos;
					while (pos < len)
					{
						if (text.mid (pos, 3) == "-->")
						{
							pos += 3;
							state = NormalState;
							break;
						}
						else
							++pos;
					}

					setFormat (start, pos - start,
							Formats_ [Comment]);
					break;
				case InTag:
					{
						QChar quote = QChar::Null;
						start = pos;
						while (pos < len)
						{
							QChar ch = text.at (pos);
							if (quote.isNull ())
							{
								if (ch == '\'' || ch == '"')
									quote = ch;
								else if (ch == '>')
								{
									++pos;
									state = NormalState;
									break;
								}
							}
							else if (ch == quote)
								quote = QChar::Null;

							++pos;
						}

						setFormat (start, pos - start,
								Formats_ [Tag]);
						break;
					}
				default:
					while (pos < len)
					{
						QChar ch = text.at (pos);
						if (ch == '<')
						{
							if (text.mid (pos, 4) == "<!--")
								state = InComment;
							else
								state = InTag;
							break;
						}
						else if (ch == '&')
						{
							start = pos;
							while (pos < len &&
									text.at (pos++) != ';') ;

							setFormat (start, pos - start,
									Formats_ [Entity]);
						}
						else
							++pos;
					}
					break;
			}
		}

		setCurrentBlockState (state);
	}

	void HtmlHighlighter::Fill ()
	{
		QTextCharFormat entityFormat;
		entityFormat.setForeground (QColor (0, 128, 0));
		entityFormat.setFontWeight (QFont::Bold);
		SetFormatFor (Entity, entityFormat);

		QTextCharFormat tagFormat;
		tagFormat.setForeground (QColor (192, 16, 112));
		tagFormat.setFontWeight (QFont::Bold);
		SetFormatFor (Tag, tagFormat);

		QTextCharFormat commentFormat;
		commentFormat.setForeground (QColor (128, 10, 74));
		commentFormat.setFontItalic (true);
		SetFormatFor (Comment, commentFormat);
	}
}
}
