/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "highlighter.h"
#include <QRegularExpression>

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
		static const QRegularExpression wordRx ("\\w+", QRegularExpression::UseUnicodePropertiesOption);
		for (const auto& match : wordRx.globalMatch (text))
		{
			const auto& word = match.captured ();
			if (word.size () <= 1)
				continue;

			const auto val = SpellcheckCache_ [word];

			const auto isCorrect = val ?
					val->IsCorrect_ :
					Checker_->IsCorrect (word);

			if (!val)
				SpellcheckCache_.insert (word, new SCResult { isCorrect }, word.size ());

			if (!isCorrect)
				setFormat (match.capturedStart (), match.capturedLength (), SpellCheckFormat_);
		}
	}
}
}
}
