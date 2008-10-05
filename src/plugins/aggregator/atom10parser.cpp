#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QtDebug>
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
	if (root.tagName () != "feed")
		return false;
	if (root.hasAttribute ("version") && root.attribute ("version") != "1.0")
		return false;
    return true;
}

channels_container_t Atom10Parser::Parse (const QDomDocument& doc) const
{
	channels_container_t channels;
    Channel_ptr chan (new Channel);
    channels.push_back (chan);

    QDomElement root = doc.documentElement ();
    chan->Title_ = root.firstChildElement ("title").text ();
	if (chan->Title_.isEmpty ())
		chan->Title_ = QObject::tr ("(No title)");
    chan->LastBuild_ = FromRFC3339 (root.firstChildElement ("updated").text ());
    chan->Link_ = GetLink (root);
    chan->Description_ = root.firstChildElement ("subtitle").text ();
    QDomElement author = root.firstChildElement ("author");
    chan->Author_ = author.firstChildElement ("name").text () + " (" + author.firstChildElement ("email").text () + ")";
    chan->Language_ = "<>";

    QDomElement entry = root.firstChildElement ("entry");
    while (!entry.isNull ())
    {
        chan->Items_.push_back (Item_ptr (ParseItem (entry)));
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
	item->Description_ = ParseEscapeAware (summary);

    return item;
}

