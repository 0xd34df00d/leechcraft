/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "atom10parser.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QtDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>

namespace LC
{
namespace Aggregator
{
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
	
	channels_container_t Atom10Parser::Parse (const QDomDocument& doc,
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
		chan->Items_ = Util::Map (Util::DomChildren (root, "entry"),
				[this, cid = chan->ChannelID_] (const QDomElement& entry) { return ParseItem (entry, cid); });

		return channels;
	}
	
	Item_ptr Atom10Parser::ParseItem (const QDomElement& entry,
			const IDType_t& channelId) const
	{
		auto item = std::make_shared<Item> (Item::CreateForChannel (channelId));

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
		GetDescription (entry, item->Description_);
	
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
