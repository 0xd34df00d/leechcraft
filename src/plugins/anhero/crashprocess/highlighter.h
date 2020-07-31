/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSyntaxHighlighter>

namespace LC
{
namespace AnHero
{
namespace CrashProcess
{
	class Highlighter : public QSyntaxHighlighter
	{
	public:
		Highlighter (QTextDocument*);

		void highlightBlock (const QString& text) override;
	private:
		void ParseBTLine (const QString&);
		void ParseFunction (const QString&, int);
		void ParseCFunction (const QString&, int);
		void ParseCppFunction (const QString&, int, int);
		void ParseRest (const QString&, int);
	};
}
}
}
