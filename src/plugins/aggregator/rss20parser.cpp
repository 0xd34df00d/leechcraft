#include <QDomDocument>
#include <QDomElement>
#include <QStringList>
#include <QLocale>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "rss20parser.h"
#include "item.h"
#include "channel.h"

RSS20Parser::RSS20Parser ()
{
	timezoneOffsets ["GMT"] = timezoneOffsets ["UT"] = timezoneOffsets ["Z"] = 0;
	timezoneOffsets ["EST"] = -5; timezoneOffsets ["EDT"] = -4;
	timezoneOffsets ["CST"] = -6; timezoneOffsets ["CDT"] = -5;
	timezoneOffsets ["MST"] = -7; timezoneOffsets ["MDT"] = -6;
	timezoneOffsets ["PST"] = -8; timezoneOffsets ["PDT"] = -7;
	timezoneOffsets ["A"] = -1; timezoneOffsets ["M"] = -12;
	timezoneOffsets ["N"] = 1; timezoneOffsets ["Y"] = +12;
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

Feed::channels_container_t RSS20Parser::Parse (const Feed::channels_container_t& channels, const QDomDocument& recent) const
{
	Feed::channels_container_t newes = Parse (recent),
        result;
    for (size_t i = 0; i < newes.size (); ++i)
    {
        boost::shared_ptr<Channel> newChannel = newes [i];
        int position = -1;
        for (size_t j = 0; j < channels.size (); ++j)
            if (*channels [j] == *newChannel)
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
            boost::shared_ptr<Channel> toInsert (new Channel ());
			toInsert->Equalify (*oldChannel);

            for (size_t j = 0; j < newChannel->Items_.size (); ++j)
            {
                bool found = false;
				size_t h = 0;
                // Check if that item already exists
                for (size_t size = oldChannel->Items_.size (); h < size; ++h)
                    if (*oldChannel->Items_ [h] == *newChannel->Items_ [j])
                    {
                        found = true;
                        break;
                    }

                if (!found)
					toInsert->Items_.push_back (newChannel->Items_ [j]);
            }
            result.push_back (toInsert);
        }
    }
    return result;
}

Feed::channels_container_t RSS20Parser::Parse (const QDomDocument& doc) const
{
	Feed::channels_container_t channels;
    QDomElement root = doc.documentElement ();
    QDomElement channel = root.firstChildElement ("channel");
    while (!channel.isNull ())
    {
        boost::shared_ptr<Channel> chan (new Channel);
        chan->Title_ = channel.firstChildElement ("title").text ();
        chan->Description_ = channel.firstChildElement ("description").text ();
        chan->Link_ = channel.firstChildElement ("link").text ();
        chan->LastBuild_ = rfc822TimeToQDateTime (channel.firstChildElement ("lastBuildDate").text ());
	if (!chan->LastBuild_.isValid () || chan->LastBuild_.isNull ())
		chan->LastBuild_ = QDateTime::currentDateTime ();
        chan->Language_ = channel.firstChildElement ("language").text ();
        chan->Author_ = channel.firstChildElement ("managingEditor").text ();
        chan->PixmapURL_ = channel.firstChildElement ("image").attribute ("url");
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
	result->Title_ = item.firstChildElement ("title").text ();
	result->Link_ = item.firstChildElement ("link").text ();
	result->Description_ = item.firstChildElement ("description").text ();
	result->PubDate_ = rfc822TimeToQDateTime (item.firstChildElement ("pubDate").text ());
	if (!result->PubDate_.isValid () || result->PubDate_.isNull ())
		result->PubDate_ = QDateTime::currentDateTime ();
	result->Guid_ = item.firstChildElement ("guid").text ();
	result->Unread_ = true;
	return result;
}

QDateTime RSS20Parser::rfc822TimeToQDateTime (const QString& t) const
{
	if (t.size () < 20)
		return QDateTime ();
	
	QString time = t.simplified ();
	short int hoursShift = 0, minutesShift = 0;
	
	if (time [3] == ',')
		time.remove (0, 5);
	QStringList tmp = time.split (' ');
	if (tmp.size () != 5)
		return QDateTime ();
	QString timezone = tmp.takeAt (tmp.size () -1);
	if (timezone.size () == 5)
	{
		bool ok;
		int tz = timezone.toInt (&ok);
		if (ok)
		{
			hoursShift = tz / 100;
			minutesShift = tz % 100;
		}
	}
	else
		hoursShift = timezoneOffsets.value (timezone,0);
	time = tmp.join (" ");
	QDateTime result;
	if ((tmp.at (2)).size () == 4)
		result = QLocale::c ().toDateTime(time, "dd MMM yyyy hh:mm:ss");
	else
		result = QLocale::c ().toDateTime(time, "dd MMM yy hh:mm:ss");
	result = result.addSecs (hoursShift*3600*(-1) + minutesShift*60*(-1));
	result.setTimeSpec(Qt::UTC);
	return result.toLocalTime ();
}
