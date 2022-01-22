/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSyntaxHighlighter>

namespace LC::LHTR
{
	class HtmlHighlighter : public QSyntaxHighlighter
	{
		enum class Construct
		{
			Entity,
			Tag,
			Comment,
			AttrName,
			AttrValue,
			MAX
		};

		QTextCharFormat Formats_ [static_cast<int> (Construct::MAX)];

		enum class State
		{
			NoState = -1,
			Normal,
			Comment,
			Tag,
			AttrName,
			AttrValue
		};
	public:
		explicit HtmlHighlighter (QTextDocument*);
	protected:
		void highlightBlock (const QString&) override;
	};
}
