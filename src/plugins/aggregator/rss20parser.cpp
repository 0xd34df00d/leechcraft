#include <boost/date_time/posix_time/posix_time.hpp>
#include <QDomDocument>
#include <QDomElement>
#include <QStringList>
#include <QtDebug>
#include "rss20parser.h"

RSS20Parser::RSS20Parser ()
{
}

RSS20Parser::~RSS20Parser ()
{
}

RSS20Parser& RSS20Parser::Instance ()
{
    static RSS20Parser inst;
    return inst;
}

bool RSS20Parser::CouldParse (const QDomDocument& doc) const
{
    QDomElement root = doc.documentElement ();
    return root.tagName () == "rss" &&
		root.attribute ("version") == "2.0";
}

channels_container_t RSS20Parser::Parse (const QDomDocument& doc) const
{
	channels_container_t channels;
    QDomElement root = doc.documentElement ();
    QDomElement channel = root.firstChildElement ("channel");
    while (!channel.isNull ())
    {
        Channel_ptr chan (new Channel);
        chan->Title_ = channel.firstChildElement ("title").text ();
        chan->Description_ = channel.firstChildElement ("description").text ();
        chan->Link_ = GetLink (channel);
        chan->LastBuild_ = RFC822TimeToQDateTime (channel.firstChildElement ("lastBuildDate").text ());
        chan->Language_ = channel.firstChildElement ("language").text ();
		chan->Author_ = GetAuthor (channel);
        if (chan->Author_.isEmpty ())
			chan->Author_ = channel.firstChildElement ("managingEditor").text ();
        if (chan->Author_.isEmpty ())
            chan->Author_ = channel.firstChildElement ("webMaster").text ();
        chan->PixmapURL_ = channel.firstChildElement ("image").attribute ("url");

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

Item* RSS20Parser::ParseItem (const QDomElement& item) const
{
	Item *result = new Item;
	result->Title_ = UnescapeHTML (item.firstChildElement ("title").text ());
	if (result->Title_.isEmpty ())
		result->Title_ = "<>";
	result->Link_ = item.firstChildElement ("link").text ();
	result->Description_ = item.firstChildElement ("description").text ();
	result->PubDate_ = RFC822TimeToQDateTime (item.firstChildElement ("pubDate").text ());
	if (!result->PubDate_.isValid () || result->PubDate_.isNull ())
		result->PubDate_ = QDateTime::currentDateTime ();
	result->Guid_ = item.firstChildElement ("guid").text ();
	if (result->Guid_.isEmpty ())
		result->Guid_ = "empty";
	result->Categories_ = GetAllCategories (item);
	result->Unread_ = true;
	result->Author_ = GetAuthor (item);
	result->NumComments_ = GetNumComments (item);
	result->CommentsLink_ = GetCommentsRSS (item);
	return result;
}

