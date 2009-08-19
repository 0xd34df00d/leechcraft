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

#ifndef PLUGINS_POSHUKU_HTMLHIGHLIGHTER_H
#define PLUGINS_POSHUKU_HTMLHIGHLIGHTER_H
#include <QSyntaxHighlighter>

namespace LeechCraft
{
	namespace Plugins
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
		};
	};
};

#endif

