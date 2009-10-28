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

#include "rss091parser.h"
#include <QDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			RSS091Parser::RSS091Parser ()
			{
			}
			
			RSS091Parser::~RSS091Parser ()
			{
			}
			
			RSS091Parser& RSS091Parser::Instance ()
			{
				static RSS091Parser inst;
				return inst;
			}
			
			bool RSS091Parser::CouldParse (const QDomDocument& doc) const
			{
				QDomElement root = doc.documentElement ();
				return root.tagName () == "rss" &&
					(root.attribute ("version") == "0.91" ||
					 root.attribute ("version") == "0.92");
			}
			
			channels_container_t RSS091Parser::Parse (const QDomDocument& doc) const
			{
				channels_container_t channels;
				QDomElement root = doc.documentElement ();
				QDomElement channel = root.firstChildElement ("channel");
				while (!channel.isNull ())
				{
					Channel_ptr chan (new Channel);
			
					chan->Title_ = channel.firstChildElement ("title").text ().trimmed ();
					chan->Description_ = channel.firstChildElement ("description").text ();
					chan->Link_ = channel.firstChildElement ("link").text ();
			
					QDomElement item = channel.firstChildElement ("item");
					while (!item.isNull ())
					{
						chan->Items_.push_back (Item_ptr (ParseItem (item)));
						item = item.nextSiblingElement ("item");
					}
					if (!chan->LastBuild_.isValid () || chan->LastBuild_.isNull ())
					{
						if (chan->Items_.size ())
							chan->LastBuild_ = chan->Items_.at (0)->PubDate_;
						else
							chan->LastBuild_ = QDateTime::currentDateTime ();
					}
			
					channels.push_back (chan);
					channel = channel.nextSiblingElement ("channel");
				}
				return channels;
			}
			
			Item* RSS091Parser::ParseItem (const QDomElement& item) const
			{
				Item *result = new Item;
				result->Title_ = UnescapeHTML (item.firstChildElement ("title").text ());
				if (result->Title_.isEmpty ())
					result->Title_ = "<>";
				result->Link_ = item.firstChildElement ("link").text ();
				result->Description_ = item.firstChildElement ("description").text ();
				result->PubDate_ = RFC822TimeToQDateTime (item.firstChildElement ("pubDate").text ());
				if (!result->PubDate_.isValid () || result->PubDate_.isNull ())
				{
					qDebug () << "Aggregator RSS 0.91: Can't parse item pubDate: " << item.firstChildElement ("pubDate").text ();
					result->PubDate_ = QDateTime::currentDateTime ();
				}
				result->Guid_ = item.firstChildElement ("guid").text ();
				if (result->Guid_.isEmpty ())
					result->Guid_ = "empty";
				result->Categories_ = GetAllCategories (item);
				result->Unread_ = true;
				result->Author_ = GetAuthor (item);
				result->NumComments_ = GetNumComments (item);
				result->CommentsLink_ = GetCommentsRSS (item);
				result->CommentsPageLink_ = GetCommentsLink (item);
				result->Enclosures_ = GetEnclosures (item);
				result->Enclosures_ += GetEncEnclosures (item);
				QPair<double, double> point = GetGeoPoint (item);
				result->Latitude_ = point.first;
				result->Longitude_ = point.second;
				return result;
			}
		};
	};
};

