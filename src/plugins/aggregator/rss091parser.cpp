/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rss091parser.h"
#include <QDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>

namespace LC
{
namespace Aggregator
{
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

	channels_container_t RSS091Parser::Parse (const QDomDocument& doc, const IDType_t& feedId) const
	{
		channels_container_t channels;
		QDomElement root = doc.documentElement ();
		for (const auto& channel : Util::DomChildren (root, "channel"))
		{
			auto chan = std::make_shared<Channel> (Channel::CreateForFeed (feedId));

			chan->Title_ = channel.firstChildElement ("title").text ().trimmed ();
			chan->Description_ = channel.firstChildElement ("description").text ();
			chan->Link_ = channel.firstChildElement ("link").text ();
			chan->Items_ = Util::Map (Util::DomChildren (channel, "item"),
					[this, cid = chan->ChannelID_] (const QDomElement& item) { return ParseItem (item, cid); });

			if (!chan->LastBuild_.isValid ())
				chan->LastBuild_ = chan->Items_.isEmpty () ?
						QDateTime::currentDateTime () :
						chan->Items_.front ()->PubDate_;

			channels.push_back (chan);
		}
		return channels;
	}

	Item_ptr RSS091Parser::ParseItem (const QDomElement& item, const IDType_t& channelId) const
	{
		auto result = std::make_shared<Item> (Item::CreateForChannel (channelId));
		result->Title_ = UnescapeHTML (item.firstChildElement ("title").text ());
		if (result->Title_.isEmpty ())
			result->Title_ = "<>";
		result->Link_ = item.firstChildElement ("link").text ();
		result->Description_ = item.firstChildElement ("description").text ();
		GetDescription (item, result->Description_);
		result->PubDate_ = RFC822TimeToQDateTime (item.firstChildElement ("pubDate").text ());
		if (!result->PubDate_.isValid () || result->PubDate_.isNull ())
		{
			qWarning () << "Aggregator RSS 0.91: Can't parse item pubDate: "
					<< item.firstChildElement ("pubDate").text ();
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
		result->Enclosures_ = GetEnclosures (item, result->ItemID_);
		result->Enclosures_ += GetEncEnclosures (item, result->ItemID_);
		QPair<double, double> point = GetGeoPoint (item);
		result->Latitude_ = point.first;
		result->Longitude_ = point.second;
		return result;
	}
}
}
