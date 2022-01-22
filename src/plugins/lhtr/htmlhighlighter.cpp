/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "htmlhighlighter.h"

namespace LC::LHTR
{
	HtmlHighlighter::HtmlHighlighter (QTextDocument *doc)
	: QSyntaxHighlighter { doc }
	{
		auto set = [this] (Construct c, const QTextCharFormat& fmt)
		{
			Formats_ [static_cast<int> (c)] = fmt;
		};
		QTextCharFormat entityFormat;
		entityFormat.setForeground ({ { 0, 128, 0 } });
		entityFormat.setFontWeight (QFont::Bold);
		set (Construct::Entity, entityFormat);

		QTextCharFormat tagFormat;
		tagFormat.setFontWeight (QFont::Bold);
		set (Construct::Tag, tagFormat);

		QTextCharFormat commentFormat;
		commentFormat.setForeground ({ { 128, 10, 74 } });
		commentFormat.setFontItalic (true);
		set (Construct::Comment, commentFormat);

		QTextCharFormat attrNameFormat;
		attrNameFormat.setForeground ({ { 10, 128, 10 } });
		set (Construct::AttrName, attrNameFormat);

		QTextCharFormat attrValueFormat;
		attrValueFormat.setForeground ({ { 128, 10, 74 } });
		set (Construct::AttrValue, attrValueFormat);
	}

	namespace
	{
		bool IsStartComment (int pos, const QString& str)
		{
			if (pos + 4 > str.size ())
				return false;

			return str.at (pos) == '<' &&
					str.at (pos + 1) == '!' &&
					str.at (pos + 2) == '-' &&
					str.at (pos + 3) == '-';
		}
	}

	void HtmlHighlighter::highlightBlock (const QString& block)
	{
		auto state = static_cast<State> (previousBlockState ());
		int start = 0;
		int pos = 0;
		const auto blockLen = block.size ();

		auto getFmt = [this] (Construct c) { return Formats_ [static_cast<int> (c)]; };

		while (pos < blockLen)
		{
			switch (state)
			{
			case State::NoState:
			case State::Normal:
				while (pos < blockLen)
				{
					const auto& ch = block.at (pos);
					if (ch == '<')
					{
						state = IsStartComment (pos, block) ? State::Comment : State::Tag;
						break;
					}
					else if (ch == '&')
					{
						start = pos;
						const auto endIdx = block.indexOf (';', pos + 1);
						if (endIdx > 0)
						{
							pos = endIdx + 1;
							setFormat (start, pos - start, getFmt (Construct::Entity));
						}
						else
							pos = blockLen;
					}
					else
						++pos;
				}
				break;
			case State::Comment:
			{
				start = pos;
				const auto endIdx = block.indexOf ("-->", pos);
				if (endIdx <= 0)
					pos = blockLen;
				else
				{
					pos = endIdx;
					state = State::Normal;
				}
				setFormat (start, pos - start, getFmt (Construct::Comment));
				break;
			}
			case State::Tag:
			{
				start = pos;
				while (pos < blockLen)
				{
					const auto ch = block.at (pos++);
					if (ch == ' ' && block.at (pos) != '/')
					{
						state = State::AttrName;
						break;
					}
					else if (ch == '>')
					{
						state = State::Normal;
						break;
					}
				}

				setFormat (start, pos - start, getFmt (Construct::Tag));
				break;
			}
			case State::AttrName:
			{
				start = pos;
				while (pos < blockLen)
				{
					const auto ch = block.at (pos++);
					if (ch == '=')
					{
						state = State::AttrValue;
						break;
					}
				}
				setFormat (start, pos - start, getFmt (Construct::AttrName));
				break;
			}
			case State::AttrValue:
			{
				start = pos;
				QChar quoteChar;
				while (pos < blockLen)
				{
					const auto ch = block.at (pos++);
					if (quoteChar == ch)
					{
						state = State::Tag;
						break;
					}
					else if (quoteChar.isNull ())
						quoteChar = ch;
				}
				setFormat (start, pos - start, getFmt (Construct::AttrValue));
				break;
			}
			}
		}

		setCurrentBlockState (static_cast<int> (state));
	}
}
