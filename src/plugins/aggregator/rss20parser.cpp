#include <QDomDocument>
#include <QtDebug>
#include <QDomElement>
#include <QStringList>
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
    QList<Item> oldes = Parse (old),
        newes = Parse (recent);
    if (oldes.size ())
    {
        Item lastItem = oldes.last ();
        int index = newes.indexOf (lastItem);
        QList<Item> result;
        for (int i = index + 1; i < newes.size (); ++i)
            result << newes.at (i);
        return result;
    }
    else
        return newes;
}

QList<Item> RSS20Parser::Parse (const QDomDocument& doc) const
{
    QList<Item> items;
    QDomElement root = doc.documentElement ();
    QDomElement channel = root.firstChildElement ("channel");
    while (!channel.isNull ())
    {
        Channel chan;
        chan.Title_ = channel.firstChildElement ("title").text ();
        chan.Description_ = channel.firstChildElement ("description").text ();
        chan.Link_ = channel.firstChildElement ("link").text ();

        QDomElement item = channel.firstChildElement ("item");
        while (!item.isNull ())
        {
            Item it = ParseItem (item);
            it.Parent_ = chan;
            items << it;
            item = item.nextSiblingElement ("item");
        }

        channel = channel.nextSiblingElement ("channel");
    }
    return items;
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

Item RSS20Parser::ParseItem (const QDomElement& item) const
{
    Item result;
    result.Title_ = UnescapeHTML (item.firstChildElement ("title").text ());
    result.Link_ = item.firstChildElement ("link").text ();
    result.Description_ = UnescapeHTML (item.firstChildElement ("description").text ());

    QString time = item.firstChildElement ("pubDate").text ();
    if (time [3] == ',')
        time.remove (0, 5);
    QStringList tmp = time.split (' ');
    tmp.removeAt (tmp.size () - 1);
    time = tmp.join (" ");
    result.PubDate_ = QDateTime::fromString (time, "d MMM yyyy hh:mm:ss");
    return result;
}

