#include <QDomDocument>
#include <QtDebug>
#include <QDomElement>
#include <QStringList>
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

QList<Channel*> RSS20Parser::Parse (const QList<Channel*>& channels, const QDomDocument& recent) const
{
    QList<Channel*> newes = Parse (recent),
        result = channels;
    for (int i = 0; i < newes.size (); ++i)
    {
        Channel *newChannel = newes.at (i);
        int position = -1;
        for (int j = 0; j < result.size (); ++j)
            if (*result.at (j) == *newChannel)
            {
                position = j;
                break;
            }
        if (position == -1)
            result.append (newChannel);
        else
        {
            Channel *oldChannel = result.at (position);
            Item *lastItemWeHave = oldChannel->Items_.last ();
            int index = 0;
            for (int j = 0; j < newChannel->Items_.size (); ++j)
                if (*newChannel->Items_.at (j) == *lastItemWeHave)
                {
                    index = j + 1;
                    break;
                }
            for (int j = index; j < newChannel->Items_.size (); ++j)
                oldChannel->Items_.append (newChannel->Items_.at (j));
        }
    }
    return result;
}

QList<Channel*> RSS20Parser::Parse (const QDomDocument& doc) const
{
    QList<Channel*> channels;
    QDomElement root = doc.documentElement ();
    QDomElement channel = root.firstChildElement ("channel");
    while (!channel.isNull ())
    {
        Channel *chan = new Channel;
        chan->Title_ = channel.firstChildElement ("title").text ();
        chan->Description_ = channel.firstChildElement ("description").text ();
        chan->Link_ = channel.firstChildElement ("link").text ();
        chan->LastBuild_ = FromRFC822 (channel.firstChildElement ("lastBuildDate").text ());

        QDomElement item = channel.firstChildElement ("item");
        while (!item.isNull ())
        {
            chan->Items_ << ParseItem (item);
            item = item.nextSiblingElement ("item");
        }

        channels << chan;

        channel = channel.nextSiblingElement ("channel");
    }
    return channels;
}

// Via
// http://www.theukwebdesigncompany.com/articles/entity-escape-characters.php
QString UnescapeHTML (const QString& escaped)
{
    QString result = escaped;
    result.replace ("&euro;", "€");
    result.replace ("&quot;", "\"");
    result.replace ("&amp;", "&");
    result.replace ("&nbsp;", " ");
    result.replace ("&lt;", "<");
    result.replace ("&gt;", ">");
    return result;
}

Item* RSS20Parser::ParseItem (const QDomElement& item) const
{
    Item *result = new Item;
    result->Title_ = UnescapeHTML (item.firstChildElement ("title").text ());
    result->Link_ = UnescapeHTML (item.firstChildElement ("link").text ());
    result->Description_ = UnescapeHTML (item.firstChildElement ("description").text ());
    result->PubDate_ = FromRFC822 (item.firstChildElement ("pubDate").text ());
    result->Guid_ = item.firstChildElement ("guid").text ();
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

