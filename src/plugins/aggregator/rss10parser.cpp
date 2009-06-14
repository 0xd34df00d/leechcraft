#include <QtDebug>
#include "rss10parser.h"

using namespace LeechCraft::Plugins::Aggregator;

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

channels_container_t RSS10Parser::Parse (const QDomDocument& doc) const
{
	channels_container_t result;

	QMap<QString, Channel_ptr> item2Channel;
	QDomElement root = doc.documentElement ();
	QDomElement channelDescr = root.firstChildElement ("channel");
	while (!channelDescr.isNull ())
	{
		Channel_ptr channel (new Channel);
		channel->Title_ = channelDescr.firstChildElement ("title").text ();
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
			Item_ptr item (new Item);
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
			item->Enclosures_ = GetEncEnclosures (itemDescr);
			if (item->Guid_.isEmpty ())
				item->Guid_ = "empty";

			item2Channel [about]->Items_.push_back (item);
		}
		itemDescr = itemDescr.nextSiblingElement ("item");
	}

	return result;
}


