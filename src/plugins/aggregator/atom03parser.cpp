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
#include "channel.h"
#include "item.h"
#include "atom03parser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Atom03Parser::Atom03Parser ()
			{
			}
			
			Atom03Parser& Atom03Parser::Instance ()
			{
				static Atom03Parser inst;
				return inst;
			}
			
			bool Atom03Parser::CouldParse (const QDomDocument& doc) const
			{
				QDomElement root = doc.documentElement ();
				if (root.tagName () != "feed")
					return false;
				if (root.hasAttribute ("version") && root.attribute ("version") == "0.3")
					return true;
				return false;
			}
			
			channels_container_t Atom03Parser::Parse (const QDomDocument& doc,
					const IDType_t& feedId) const
			{
				channels_container_t channels;
				Channel_ptr chan (new Channel (feedId));
				channels.push_back (chan);
			
				QDomElement root = doc.documentElement ();
				chan->Title_ = root.firstChildElement ("title").text ().trimmed ();
				if (chan->Title_.isEmpty ())
					chan->Title_ = QObject::tr ("(No title)");
				chan->LastBuild_ = FromRFC3339 (root.firstChildElement ("updated").text ());
				chan->Link_ = GetLink (root);
				chan->Description_ = root.firstChildElement ("tagline").text ();
				chan->Language_ = "<>";
				chan->Author_ = GetAuthor (root);
			
				QDomElement entry = root.firstChildElement ("entry");
				while (!entry.isNull ())
				{
					chan->Items_.push_back (Item_ptr (ParseItem (entry, chan->ChannelID_)));
					entry = entry.nextSiblingElement ("entry");
				}
			
				return channels;
			}
			
			Item* Atom03Parser::ParseItem (const QDomElement& entry,
					const IDType_t& channelId) const
			{
				Item *item = new Item (channelId);
			
				item->Title_ = ParseEscapeAware (entry.firstChildElement ("title"));
				item->Link_ = GetLink (entry);
				item->Guid_ = entry.firstChildElement ("id").text ();
				item->Unread_ = true;
			
				QDomElement date = entry.firstChildElement ("modified");
				if (date.isNull ())
					date = entry.firstChildElement ("issued");
				item->PubDate_ = FromRFC3339 (date.text ());
			
				QDomElement summary = entry.firstChildElement ("content");
				if (summary.isNull ())
					summary = entry.firstChildElement ("summary");
				item->Description_ = ParseEscapeAware (summary);
			
				item->Categories_ += GetAllCategories (entry);
				item->Author_ = GetAuthor (entry);
			
				item->NumComments_ = GetNumComments (entry);
				item->CommentsLink_ = GetCommentsRSS (entry);
				item->CommentsPageLink_ = GetCommentsLink (entry);
			
				item->Enclosures_ = GetEnclosures (entry, item->ItemID_);
				item->Enclosures_ += GetEncEnclosures (entry, item->ItemID_);

				QPair<double, double> point = GetGeoPoint (entry);
				item->Latitude_ = point.first;
				item->Longitude_ = point.second;
				item->MRSSEntries_ = GetMediaRSS (entry);
			
				return item;
			}
		};
	};
};

