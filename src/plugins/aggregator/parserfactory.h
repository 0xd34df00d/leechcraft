#ifndef PARSERFACTORY_H
#define PARSERFACTORY_H
#include <QList>

class QDomDocument;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class Parser;

			class ParserFactory
			{
			    QList<Parser*> Parsers_;
			    ParserFactory ();
			public:
			    static ParserFactory& Instance ();
			    void Register (Parser*);
			    Parser* Return (const QDomDocument&) const;
			};
		};
	};
};

#endif

