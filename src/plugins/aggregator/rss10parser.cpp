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

#include <QtDebug>
#include "rss10parser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			RSS10Parser::RSS10Parser ()
			{
			}
			
			RSS10Parser::~RSS10Parser ()
			{
			}
			
			RSS10Parser& RSS10Parser::Instance ()
			{
				static RSS10Parser inst;
				return inst;
			}
			
			bool RSS10Parser::CouldParse (const QDomDocument& doc) const
			{
				QDomElement root = doc.documentElement ();
				return root.tagName () == "RDF";
			}
			
			channels_container_t RSS10Parser::Parse (const QDomDocument& doc,
					const IDType_t& feedId) const
			{
				channels_container_t result;
			
				QMap<QString, Channel_ptr> item2Channel;
				QDomElement root = doc.documentElement ();
				QDomElement channelDescr = root.firstChildElement ("channel");
				while (!channelDescr.isNull ())
				{
					Channel_ptr channel (new Channel (feedId));
					channel->Title_ = channelDescr.firstChildElement ("title").text ().trimmed ();
					channel->Link_ = channelDescr.firstChildElement ("link").text ();
					channel->Description_ =
						channelDescr.firstChildElement ("description").text ();
					channel->PixmapURL_ =
						channelDescr.firstChildElement ("image")
						.firstChildElement ("url").text ();
					channel->LastBuild_ = GetDCDateTime (channelDescr);
			
					QDomElement itemsRoot = channelDescr.firstChildElement ("items");
					QDomNodeList seqs = itemsRoot.elementsByTagNameNS (RDF_, "Seq");
			
					channelDescr = channelDescr.nextSiblingElement ("channel");
			
					if (!seqs.size ())
						continue;
			
					QDomElement seqElem = seqs.at (0).toElement ();
					QDomNodeList lis = seqElem.elementsByTagNameNS (RDF_, "li");
					for (int i = 0; i < lis.size (); ++i)
						item2Channel [lis.at (i).toElement ().attribute ("resource")] = channel;
			
					result.push_back (channel);
				}
			
				QDomElement itemDescr = root.firstChildElement ("item");
				while (!itemDescr.isNull ())
				{
					QString about = itemDescr.attributeNS (RDF_, "about");
					if (item2Channel.contains (about))
					{
						Item_ptr item (new Item (item2Channel [about]->ChannelID_));
						item->Title_ = itemDescr.firstChildElement ("title").text ();
						item->Link_ = itemDescr.firstChildElement ("link").text ();
						item->Description_ = itemDescr.firstChildElement ("description").text ();
			
						item->Categories_ = GetAllCategories (itemDescr);
						item->Author_ = GetAuthor (itemDescr);
						item->PubDate_ = GetDCDateTime (itemDescr);
						item->Unread_ = true;
						item->NumComments_ = GetNumComments (itemDescr);
						item->CommentsLink_ = GetCommentsRSS (itemDescr);
						item->CommentsPageLink_ = GetCommentsLink (itemDescr);
						item->Enclosures_ = GetEncEnclosures (itemDescr, item->ItemID_);
						QPair<double, double> point = GetGeoPoint (itemDescr);
						item->Latitude_ = point.first;
						item->Longitude_ = point.second;
						if (item->Guid_.isEmpty ())
							item->Guid_ = "empty";
			
						item2Channel [about]->Items_.push_back (item);
					}
					itemDescr = itemDescr.nextSiblingElement ("item");
				}
			
				return result;
			}
		};
	};
};

