#include <QDomDocument>
#include <QtDebug>
#include <QDomElement>
#include <QStringList>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "rss20parser.h"
#include "item.h"
#include "channel.h"

RSS20Parser::RSS20Parser ()
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
    return root.tagName () == "rss" && root.attribute ("version") == "2.0";
}

std::vector<boost::shared_ptr<Channel> > RSS20Parser::Parse (const std::vector<boost::shared_ptr<Channel> >& channels, const QDomDocument& recent) const
{
    std::vector<boost::shared_ptr<Channel> > newes = Parse (recent),
        result;
    for (int i = 0; i < newes.size (); ++i)
    {
        boost::shared_ptr<Channel> newChannel = newes.at (i);
        int position = -1;
        for (int j = 0; j < channels.size (); ++j)
            if (*channels.at (j) == *newChannel)
            {
                position = j;
                break;
            }
        if (position == -1)
            result.push_back (newChannel);
        else if (!channels [position]->Items_.size ())
        {
            boost::shared_ptr<Channel> pointer = channels [position];
            pointer->Items_ = newChannel->Items_;
            result.push_back (pointer);
        }
        else
        {
            boost::shared_ptr<Channel> oldChannel = channels [position];
            boost::shared_ptr<Channel> toInsert (new Channel);
            *toInsert = *oldChannel;
            toInsert->LastBuild_ = newChannel->LastBuild_;

            for (int j = 0; j < newChannel->Items_.size (); ++j)
            {
                bool found = false;
                // Check if that item already exists
                for (int h = 0; h < toInsert->Items_.size (); ++h)
                    if (*toInsert->Items_ [h] == *newChannel->Items_ [j])
                    {
                        found = true;
                        break;
                    }
                if (found)
                    continue;

                // Okay, this item is new, let's find where to place
                // it. We should place it before the first found item
                // with earlier datetime.
                for (int h = 0; h < toInsert->Items_.size (); ++h)
                {
                    if (toInsert->Items_ [h]->PubDate_ < newChannel->Items_ [j]->PubDate_)
                    {
                        toInsert->Items_.insert (toInsert->Items_.begin () + h++, newChannel->Items_ [j]);
                        break;
                    }
                }
            }
            result.push_back (toInsert);
        }
    }
    return result;
}

std::vector<boost::shared_ptr<Channel> > RSS20Parser::Parse (const QDomDocument& doc) const
{
    std::vector<boost::shared_ptr<Channel> > channels;
    QDomElement root = doc.documentElement ();
    QDomElement channel = root.firstChildElement ("channel");
    while (!channel.isNull ())
    {
        boost::shared_ptr<Channel> chan (new Channel);
        chan->Title_ = channel.firstChildElement ("title").text ();
        chan->Description_ = channel.firstChildElement ("description").text ();
        chan->Link_ = channel.firstChildElement ("link").text ();
        chan->LastBuild_ = FromRFC822 (channel.firstChildElement ("lastBuildDate").text ());
        chan->Language_ = channel.firstChildElement ("language").text ();
        chan->Author_ = channel.firstChildElement ("managingEditor").text ();
        if (chan->Author_.isEmpty ())
            chan->Author_ = channel.firstChildElement ("webMaster").text ();

        QDomElement item = channel.firstChildElement ("item");
        while (!item.isNull ())
        {
            chan->Items_.push_back (boost::shared_ptr<Item> (ParseItem (item)));
            item = item.nextSiblingElement ("item");
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
    result->Link_ = UnescapeHTML (item.firstChildElement ("link").text ());
    result->Description_ = UnescapeHTML (item.firstChildElement ("description").text ());
    result->PubDate_ = FromRFC822 (item.firstChildElement ("pubDate").text ());
    if (!result->PubDate_.isValid () || result->PubDate_.isNull ())
        result->PubDate_ = QDateTime::currentDateTime ();
    result->Guid_ = item.firstChildElement ("guid").text ();
    result->Unread_ = true;
    return result;
}

QDateTime RSS20Parser::FromRFC822 (const QString& t) const
{
    QString time = t;
    if (time [3] == ',')
        time.remove (0, 5);
    QStringList tmp = time.split (' ');
    tmp.removeAt (tmp.size () - 1);
    time = tmp.join (" ");
    return QDateTime::fromString (time, "d MMM yyyy hh:mm:ss");
}

