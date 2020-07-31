/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QSyntaxHighlighter>
#include <QTextFormat>
#include <QCache>
#include <interfaces/ispellcheckprovider.h>

namespace LC
{
namespace Azoth
{
namespace Rosenthal
{
	class Highlighter : public QSyntaxHighlighter
	{
		QTextCharFormat SpellCheckFormat_;
		const ISpellChecker_ptr Checker_;

		struct SCResult
		{
			bool IsCorrect_;
		};
		QCache<QString, SCResult> SpellcheckCache_;
	public:
		Highlighter (const ISpellChecker_ptr&, QTextDocument*);
	protected:
		void highlightBlock (const QString&);
	};
}
}
}
