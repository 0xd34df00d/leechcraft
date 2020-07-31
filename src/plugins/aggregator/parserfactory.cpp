/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QtDebug>
#include "parserfactory.h"
#include "parser.h"
#include "rss20parser.h"
#include "rss10parser.h"
#include "rss091parser.h"
#include "atom10parser.h"
#include "atom03parser.h"

namespace LC
{
namespace Aggregator
{
	ParserFactory& ParserFactory::Instance ()
	{
		static ParserFactory inst;
		return inst;
	}
	
	void ParserFactory::Register (Parser *parser)
	{
		Parsers_.append (parser);
	}

	void ParserFactory::RegisterDefaultParsers ()
	{
		Register (&RSS20Parser::Instance ());
		Register (&Atom10Parser::Instance ());
		Register (&RSS091Parser::Instance ());
		Register (&Atom03Parser::Instance ());
		Register (&RSS10Parser::Instance ());
	}
	
	Parser* ParserFactory::Return (const QDomDocument& doc) const
	{
		for (auto parser : Parsers_)
			if (parser->CouldParse (doc))
				return parser;
		return nullptr;
	}
}
}

