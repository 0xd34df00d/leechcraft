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

