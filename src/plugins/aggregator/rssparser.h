/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_RSSPARSER_H
#define PLUGINS_AGGREGATOR_RSSPARSER_H
#include <QMap>
#include <QString>
#include "parser.h"
#include "channel.h"

class QDomDocument;

namespace LC
{
namespace Aggregator
{
	class RSSParser : public Parser
	{
	protected:
		QMap<QString, int> TimezoneOffsets_;

		RSSParser ();
	public:
		virtual ~RSSParser ();
	protected:
		QDateTime RFC822TimeToQDateTime (const QString&) const;
		QList<Enclosure> GetEnclosures (const QDomElement&, const IDType_t&) const;
	};
}
}

#endif
