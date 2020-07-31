/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_ATOMPARSER_H
#define PLUGINS_AGGREGATOR_ATOMPARSER_H
#include "parser.h"
#include "channel.h"

class QDomDocument;
class QString;

namespace LC
{
namespace Aggregator
{
	class AtomParser : public Parser
	{
	protected:
		AtomParser ();
	public:
		virtual ~AtomParser ();
	protected:
		virtual QString ParseEscapeAware (const QDomElement&) const;
		QList<Enclosure> GetEnclosures (const QDomElement&,
				const IDType_t&) const;
	};
}
}

#endif
