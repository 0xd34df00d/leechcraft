/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "highlighter.h"

namespace LC
{
namespace Azoth
{
namespace Rosenthal
{
	Highlighter::Highlighter (const ISpellChecker_ptr& checker, QTextDocument *parent)
	: QSyntaxHighlighter { parent }
	, Checker_ { checker }
	, SpellcheckCache_ { 50 * 1024 }
	{
		SpellCheckFormat_.setUnderlineColor ({ Qt::red });
		SpellCheckFormat_.setUnderlineStyle (QTextCharFormat::SpellCheckUnderline);
	}

	void Highlighter::highlightBlock (const QString& text)
	{
		QRegularExpression sr ("\\W+");
		const auto& splitted = text.split (sr, Qt::SkipEmptyParts);
		int prevStopPos = 0;
		for (const auto& str : splitted)
		{
			if (str.size () <= 1)
				continue;

			const auto val = SpellcheckCache_ [str];

			const auto isCorrect = val ?
					val->IsCorrect_ :
					Checker_->IsCorrect (str);

			if (!val)
				SpellcheckCache_.insert (str, new SCResult { isCorrect }, str.size ());

			if (isCorrect)
				continue;

			const int pos = text.indexOf (str, prevStopPos);
			if (pos >= 0)
			{
				setFormat (pos, str.length (), SpellCheckFormat_);
				prevStopPos = pos + str.length ();
			}
		}
	}
}
}
}
