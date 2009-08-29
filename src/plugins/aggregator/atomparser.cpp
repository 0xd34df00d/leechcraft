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

#include "atomparser.h"
#include <QDomDocument>
#include <QString>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			AtomParser::AtomParser ()
			{
			}

			AtomParser::~AtomParser ()
			{
			}

			QString AtomParser::ParseEscapeAware (const QDomElement& parent) const
			{
				QString result;
				if (!parent.hasAttribute ("type") ||
						parent.attribute ("type") == "text" ||
						(parent.attribute ("type") == "text/html" &&
						 parent.attribute ("mode") != "escaped"))
					result = parent.text ();
				else if (parent.attribute ("type") == "text/html" &&
						parent.attribute ("mode") == "escaped")
					result = UnescapeHTML (parent.text ());
				else
					result = UnescapeHTML (parent.text ());
			
				return result;
			}
			
			QList<Enclosure> AtomParser::GetEnclosures (const QDomElement& entry) const
			{
				QList<Enclosure> result;
				QDomNodeList links = entry.elementsByTagName ("link");
				for (int i = 0; i < links.size (); ++i)
				{
					QDomElement link = links.at (i).toElement ();
					if (link.attribute ("rel") != "enclosure")
						continue;
			
					Enclosure e =
					{
						link.attribute ("href"),
						link.attribute ("type"),
						link.attribute ("length", "-1").toLongLong (),
						link.attribute ("hreflang")
					};
			
					result << e;
				}
				return result;
			}
		};
	};
};

