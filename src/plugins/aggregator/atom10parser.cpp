#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QString>
#include <QtDebug>
#include "channel.h"
#include "item.h"
#include "atom10parser.h"

Atom10Parser::Atom10Parser ()
{
}

Atom10Parser& Atom10Parser::Instance ()
{
    static Atom10Parser inst;
    return inst;
}

bool Atom10Parser::CouldParse (const QDomDocument& doc) const
{
    QDomElement root = doc.documentElement ();
    return root.tagName () == "feed";
}

QList<Channel*> Atom10Parser::Parse (const QList<Channel*>& old, const QDomDocument& recent) const
{
    QList<Channel*> newes = Parse (recent),
        result;
    if (!newes.size ())
        return QList<Channel*> ();
    else if (!old.size ())
        return newes;
    else
    {
        Channel *toInsert = new Channel;
        *toInsert = *newes.at (0);
        result.append (toInsert);
        result.at (0)->Items_.clear ();
        Item *lastItemWeHave = old.at (0)->Items_.first ();
        int index = newes.at (0)->Items_.size ();
        for (int j = 0; j < newes.at (0)->Items_.size (); ++j)
            if (*newes.at (0)->Items_.at (j) == *lastItemWeHave)
            {
                index = j - 1;
                break;
            }
        for (int j = index; j >= 0; --j)
            result.at (0)->Items_.prepend (newes.at (0)->Items_.at (j));
    }
    return result;
}

QList<Channel*> Atom10Parser::Parse (const QDomDocument& doc) const
{
    QList<Channel*> channels;
    Channel *chan = new Channel;
    channels.append (chan);

    QDomElement root = doc.documentElement ();
    chan->Title_ = root.firstChildElement ("title").text ();
    chan->LastBuild_ = FromRFC3339 (root.firstChildElement ("updated").text ());
    chan->Link_ = GetLink (root);

    QDomElement entry = root.firstChildElement ("entry");
    while (!entry.isNull ())
    {
        chan->Items_.append (ParseItem (entry));

        entry = entry.nextSiblingElement ("entry");
    }

    return channels;
}

Item* Atom10Parser::ParseItem (const QDomElement& entry) const
{
    Item *item = new Item;

    item->Title_ = entry.firstChildElement ("title").text ();
    item->Link_ = GetLink (entry);
    item->Guid_ = entry.firstChildElement ("id").text ();
    item->PubDate_ = FromRFC3339 (entry.firstChildElement ("updated").text ());
    item->Unread_ = true;

    QDomElement summary = entry.firstChildElement ("content");
    if (summary.isNull ())
        summary = entry.firstChildElement ("summary");
    if (!summary.hasAttribute ("type") || summary.attribute ("type") == "text")
        item->Description_ = summary.text ();
    else
        item->Description_ = UnescapeHTML (summary.text ());

    return item;
}

QDateTime Atom10Parser::FromRFC3339 (const QString& t) const
{
    return QDateTime::fromString (t.left (19), "yyyy-MM-ddTHH:mm:ss");
}

QString Atom10Parser::GetLink (const QDomElement& parent) const
{
    QString result;
    QDomElement link = parent.firstChildElement ("link");
    while (!link.isNull ())
    {
        if (!link.hasAttribute ("rel") || link.attribute ("rel") == "alternate")
            result = link.attribute ("href");
        link = link.nextSiblingElement ("link");
    }
    return result;
}

