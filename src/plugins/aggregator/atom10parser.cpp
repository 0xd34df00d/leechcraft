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

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QtDebug>
#include "atom10parser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Atom10Parser::Atom10Parser ()
			{
			}
			
			Atom10Parser& Atom10Parser::Instance ()
			{
				static Atom10Parser inst;
				return inst;
			}
			
			bool Atom10Parser::CouldParse (const QDomDocument& doc) const
			{
				QDomElement root = doc.documentElement ();
				if (root.tagName () != "feed")
					return false;
				if (root.hasAttribute ("version") && root.attribute ("version") != "1.0")
					return false;
				return true;
			}
			
			channels_container_t Atom10Parser::Parse (const QDomDocument& doc) const
			{
				channels_container_t channels;
				Channel_ptr chan (new Channel);
				channels.push_back (chan);
			
				QDomElement root = doc.documentElement ();
				chan->Title_ = root.firstChildElement ("title").text ();
				if (chan->Title_.isEmpty ())
					chan->Title_ = QObject::tr ("(No title)");
				chan->LastBuild_ = FromRFC3339 (root.firstChildElement ("updated").text ());
				chan->Link_ = GetLink (root);
				chan->Description_ = root.firstChildElement ("subtitle").text ();
				chan->Author_ = GetAuthor (root);
				if (chan->Author_.isEmpty ())
				{
					QDomElement author = root.firstChildElement ("author");
					chan->Author_ = author.firstChildElement ("name").text () +
						" (" +
						author.firstChildElement ("email").text () +
						")";
				}
				chan->Language_ = "<>";
			
				QDomElement entry = root.firstChildElement ("entry");
				while (!entry.isNull ())
				{
					chan->Items_.push_back (Item_ptr (ParseItem (entry)));
					entry = entry.nextSiblingElement ("entry");
				}
			
				return channels;
			}
			
			Item* Atom10Parser::ParseItem (const QDomElement& entry) const
			{
				Item *item = new Item;
			
				item->Title_ = entry.firstChildElement ("title").text ();
				item->Link_ = GetLink (entry);
				item->Guid_ = entry.firstChildElement ("id").text ();
				item->PubDate_ = FromRFC3339 (entry.firstChildElement ("updated").text ());
				item->Unread_ = true;
				item->Categories_ = GetAllCategories (entry);
				item->Author_ = GetAuthor (entry);
				item->NumComments_ = GetNumComments (entry);
				item->CommentsLink_ = GetCommentsRSS (entry);
				item->CommentsPageLink_ = GetCommentsLink (entry);
			
				QDomElement summary = entry.firstChildElement ("content");
				if (summary.isNull ())
					summary = entry.firstChildElement ("summary");
				item->Description_ = ParseEscapeAware (summary);
			
				item->Enclosures_ = GetEnclosures (entry);
				item->Enclosures_ += GetEncEnclosures (entry);

				QPair<double, double> point = GetGeoPoint (entry);
				item->Latitude_ = point.first;
				item->Longitude_ = point.second;
			
				return item;
			}
		};
	};
};

