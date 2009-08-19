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

#include "opmlparser.h"
#include <QDomDocument>
#include <QDomElement>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			OPMLParser::OPMLParser (const QDomDocument& document)
			: CacheValid_ (false)
			, Document_ (document)
			{
			}
			
			void OPMLParser::Reset (const QDomDocument& document)
			{
				Document_ = document;
				CacheValid_ = false;
			}
			
			bool OPMLParser::IsValid () const
			{
				QDomElement root = Document_.documentElement ();
				if (root.tagName () != "opml")
					return false;
			
				QDomNodeList heads = root.elementsByTagName ("head");
				if (heads.size () != 1 || !heads.at (0).isElement ())
					return false;
			
				QDomNodeList bodies = root.elementsByTagName ("body");
				if (bodies.size () != 1 || !bodies.at (0).isElement ())
					return false;
			
				if (!bodies.at (0).toElement ().elementsByTagName ("outline").size ())
					return false;
			
				return true;
			}
			
			OPMLParser::OPMLinfo_t OPMLParser::GetInfo () const
			{
				OPMLinfo_t result;
			
				QDomNodeList entries = Document_.documentElement ().firstChildElement ("head").childNodes ();
			
				for (int i = 0; i < entries.size (); ++i)
				{
					QDomElement elem = entries.at (i).toElement ();
					if (elem.isNull ())
						continue;
			
					result [elem.tagName ()] = elem.text ();
				}
			
				return result;
			}
			
			OPMLParser::items_container_t OPMLParser::Parse () const
			{
				if (!CacheValid_)
				{
					Items_.clear ();
			
					QDomElement body = Document_.documentElement ()
						.firstChildElement ("body");
					QDomElement outline = body.firstChildElement ("outline");
					while (!outline.isNull ())
					{ 
						ParseOutline (outline);
						outline = outline.nextSiblingElement ("outline");
					}
			
					CacheValid_ = true;
				}
			
				return Items_;
			}
			
			void OPMLParser::ParseOutline (const QDomElement& parentOutline,
					QStringList previousStrings) const
			{
				if (parentOutline.hasAttribute ("xmlUrl"))
				{
					OPMLItem item;
					item.URL_ = parentOutline.attribute ("xmlUrl");
					item.HTMLUrl_ = parentOutline.attribute ("htmlUrl");
					item.Title_ = parentOutline.attribute ("title");
					item.CustomFetchInterval_ = (parentOutline
						.attribute ("useCustomFetchInterval") == "true");
					item.MaxArticleAge_ = parentOutline
						.attribute ("maxArticleAge").toInt ();
					item.FetchInterval_ = parentOutline
						.attribute ("fetchInterval").toInt ();
					item.MaxArticleNumber_ = parentOutline
						.attribute ("maxArticleNumber").toInt ();
					item.Description_ = parentOutline.attribute ("description");
					item.Categories_ = previousStrings;
			
					Items_.push_back (item);
				}
			
				if (parentOutline.attribute ("text").size ())
					previousStrings << parentOutline.attribute ("text");
			
				QDomElement outline = parentOutline.firstChildElement ("outline");
				while (!outline.isNull ())
				{
					ParseOutline (outline, previousStrings);
					outline = outline.nextSiblingElement ("outline");
				}
			}
		};
	};
};

