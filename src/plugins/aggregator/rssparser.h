/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_RSSPARSER_H
#define PLUGINS_AGGREGATOR_RSSPARSER_H
#include <QMap>
#include <QString>
#include "parser.h"
#include "channel.h"

class QDomDocument;

namespace LeechCraft
{
	namespace Plugins
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
		};
	};
};

#endif

