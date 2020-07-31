/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "atom03parser.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QtDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include "channel.h"
#include "item.h"

namespace LC
{
namespace Aggregator
{
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
		auto chan = std::make_shared<Channel> (Channel::CreateForFeed (feedId));
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
		chan->Items_ = Util::Map (Util::DomChildren (root, "entry"),
				[this, cid = chan->ChannelID_] (const QDomElement& entry) { return ParseItem (entry, cid); });

		return channels;
	}
	
	Item_ptr Atom03Parser::ParseItem (const QDomElement& entry, const IDType_t& channelId) const
	{
		auto item = std::make_shared<Item> (Item::CreateForChannel (channelId));

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
		GetDescription (entry, item->Description_);

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
		item->MRSSEntries_ = GetMediaRSS (entry, item->ItemID_);

		return item;
	}
}
}
