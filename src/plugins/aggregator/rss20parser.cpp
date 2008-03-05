#include <QDomDocument>
#include <QtDebug>
#include <QDomElement>
#include "rss20parser.h"

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

QList<Item> RSS20Parser::Parse (const QDomDocument& old, const QDomDocument& recent) const
{
    QPair<QList<Channel>, QList<Item> > oldes = Parse (old),
        newes = Parse (recent);
    if (oldes.second.size ())
    {
        Item lastItem = oldes.second.last ();
        int index = newes.second.indexOf (lastItem);
        QList<Item> result;
        for (int i = index; i < newes.second.size (); ++i)
            result << newes.second.at (i);
        return result;
    }
    else
        return newes.second;
}

QPair<QList<Channel>, QList<Item> > RSS20Parser::Parse (const QDomDocument& doc) const
{
    QList<Channel> channels;
    QList<Item> items;
    QDomElement root = doc.documentElement ();
    QDomElement channel = root.firstChildElement ("channel");
    while (!channel.isNull ())
    {
        Channel chan;
        chan.Title_ = channel.firstChildElement ("title").text ();
        chan.Description_ = channel.firstChildElement ("description").text ();
        chan.Link_ = channel.firstChildElement ("link").text ();
        channels << chan;

        QDomElement item = channel.firstChildElement ("item");
        while (!item.isNull ())
        {
            Item it = ParseItem (item);
            it.Parent_ = &chan;
            items << it;
            item = item.nextSiblingElement ("item");
        }

        channel = channel.nextSiblingElement ("channel");
    }
    return qMakePair (channels, items);
}

Item RSS20Parser::ParseItem (const QDomElement& item) const
{
    Item result;
    result.Title_ = item.firstChildElement ("title").text ();
    return result;
}

