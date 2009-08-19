/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "htmlhighlighter.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			HtmlHighlighter::HtmlHighlighter (QTextDocument *doc)
			: QSyntaxHighlighter (doc)
			{
				Fill ();
			}
			
			HtmlHighlighter::HtmlHighlighter (QTextEdit *edit)
			: QSyntaxHighlighter (edit)
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
		};
	};
};

