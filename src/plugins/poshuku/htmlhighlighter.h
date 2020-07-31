/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSyntaxHighlighter>

class QTextEdit;

namespace LC
{
namespace Poshuku
{
	/* This class is based on the example found on:
	 * http://www.crossplatform.ru/node/312
	 * My thankful rays go to the author.
	 */
	class HtmlHighlighter : public QSyntaxHighlighter
	{
		Q_OBJECT

	public:
		enum Construct
		{
			Entity,
			Tag,
			Comment,
			LastConstruct = Comment
		};
	private:
		QTextCharFormat Formats_ [LastConstruct + 1];
	public:
		HtmlHighlighter (QTextDocument*);
		HtmlHighlighter (QTextEdit*);
		void SetFormatFor (Construct, const QTextCharFormat&);
		QTextCharFormat GetFormatFor (Construct) const;
	protected:
		enum State
		{
			NormalState = -1,
			InComment,
			InTag
		};
		void highlightBlock (const QString&);
	private:
		void Fill ();
	};
}
}
