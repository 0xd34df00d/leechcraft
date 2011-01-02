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

#ifndef PLUGINS_AGGREGATOR_ATOM10PARSER_H
#define PLUGINS_AGGREGATOR_ATOM10PARSER_H
#include <QPair>
#include <QDateTime>
#include "atomparser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class Atom10Parser : public AtomParser
			{
				Atom10Parser ();
			public:
				static Atom10Parser& Instance ();
				virtual bool CouldParse (const QDomDocument&) const;
			private:
				channels_container_t Parse (const QDomDocument&,
						const IDType_t&) const;
				Item* ParseItem (const QDomElement&,
						const IDType_t&) const;
			};
		};
	};
};

#endif

