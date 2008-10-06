#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QtDebug>
#include "channel.h"
#include "item.h"
#include "atom03parser.h"

Atom03Parser::Atom03Parser ()
{
}

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

channels_container_t Atom03Parser::Parse (const QDomDocument& doc) const
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
    chan->Description_ = root.firstChildElement ("tagline").text ();
    chan->Language_ = "<>";
	chan->Author_ = GetAuthor (root);

    QDomElement entry = root.firstChildElement ("entry");
    while (!entry.isNull ())
    {
        chan->Items_.push_back (Item_ptr (ParseItem (entry)));
        entry = entry.nextSiblingElement ("entry");
    }

    return channels;
}

Item* Atom03Parser::ParseItem (const QDomElement& entry) const
{
    Item *item = new Item;

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

	item->Categories_ += GetAllCategories (entry);
	item->Author_ = GetAuthor (entry);

    return item;
}

