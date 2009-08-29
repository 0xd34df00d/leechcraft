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

#ifndef PLUGINS_AGGREGATOR_ATOMPARSER_H
#define PLUGINS_AGGREGATOR_ATOMPARSER_H
#include "parser.h"
#include "channel.h"

class QDomDocument;
class QString;

namespace LeechCraft
{
	namespace Plugins
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
				QList<Enclosure> GetEnclosures (const QDomElement&) const;
			};
		};
	};
};

#endif

