#include <QtDebug>
#include "parserfactory.h"
#include "parser.h"

using namespace LeechCraft::Plugins::Aggregator;

ParserFactory::ParserFactory ()
{
}

ParserFactory& ParserFactory::Instance ()
{
	static ParserFactory inst;
	return inst;
}

void ParserFactory::Register (Parser *parser)
{
	Parsers_.append (parser);
}

Parser* ParserFactory::Return (const QDomDocument& doc) const
{
	Parser *result = 0;
	for (int i = 0; i < Parsers_.size (); ++i)
		if (Parsers_.at (i)->CouldParse (doc))
		{
			result = Parsers_ [i];
			break;
		}
	return result;
}


