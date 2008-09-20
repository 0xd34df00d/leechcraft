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

Feed::channels_container_t Atom10Parser::Parse (const Feed::channels_container_t& old, const QDomDocument& recent) const
{
	Feed::channels_container_t newes = Parse (recent),
        result;
    if (!newes.size ())
        return Feed::channels_container_t ();
    else if (!old.size ())
        return newes;
    else
    {
        boost::shared_ptr<Channel> toInsert (new Channel ());
        toInsert->Equalify (*old [0]);
        boost::shared_ptr<Item> lastItemWeHave = old [0]->Items_ [0];
        int index = newes [0]->Items_.size ();
        for (size_t j = 0, size = newes [0]->Items_.size (); j < size; ++j)
            if (*newes [0]->Items_ [j] == *lastItemWeHave)
            {
                index = j - 1;
                break;
            }
        for (int j = index; j >= 0; --j)
            toInsert->Items_.insert (toInsert->Items_.begin (), newes [0]->Items_ [j]);

        result.push_back (toInsert);
    }
    return result;
}

Feed::channels_container_t Atom10Parser::Parse (const QDomDocument& doc) const
{
	Feed::channels_container_t channels;
    boost::shared_ptr<Channel> chan (new Channel);
    channels.push_back (chan);

    QDomElement root = doc.documentElement ();
    chan->Title_ = root.firstChildElement ("title").text ();
	if (chan->Title_.isEmpty ())
		chan->Title_ = tr ("(No title)");
    chan->LastBuild_ = FromRFC3339 (root.firstChildElement ("updated").text ());
    chan->Link_ = GetLink (root);
    chan->Description_ = root.firstChildElement ("subtitle").text ();
    QDomElement author = root.firstChildElement ("author");
    chan->Author_ = author.firstChildElement ("name").text () + " (" + author.firstChildElement ("email").text () + ")";
    chan->Language_ = "<>";

    QDomElement entry = root.firstChildElement ("entry");
    while (!entry.isNull ())
    {
        chan->Items_.push_back (boost::shared_ptr<Item> (ParseItem (entry)));
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
	int hoursShift = 0, minutesShift = 0;
	if (t.size () < 19)
		return QDateTime ();
	QDateTime result = QDateTime::fromString (t.left (19).toUpper (), "yyyy-MM-ddTHH:mm:ss");
	QRegExp fractionalSeconds ("(\\.)(\\d+)");
	if (fractionalSeconds.indexIn (t) > -1)
	{
		bool ok;
		int fractional = fractionalSeconds.cap (2).toInt (&ok);
		if (ok)
		{
			if (fractional < 100)
				fractional *= 10;
			if (fractional <10) 
				fractional *= 100;
			result.addMSecs (fractional);
		}
	}
	QRegExp timeZone ("(\\+|\\-)(\\d\\d)(:)(\\d\\d)$");
	if (timeZone.indexIn (t) > -1)
	{
		short int multiplier = -1;
		if (timeZone.cap (1) == "-")
			multiplier = 1;
		hoursShift = timeZone.cap (2).toInt ();
		minutesShift = timeZone.cap (4).toInt ();
		result = result.addSecs (hoursShift * 3600 * multiplier + minutesShift * 60 * multiplier);
	}
	result.setTimeSpec (Qt::UTC);
	return result.toLocalTime ();
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

