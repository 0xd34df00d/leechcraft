#include "rssparser.h"
#include <QDomDocument>
#include <QLocale>

RSSParser::RSSParser ()
{
	TimezoneOffsets_ ["GMT"] = TimezoneOffsets_ ["UT"] = TimezoneOffsets_ ["Z"] = 0;
	TimezoneOffsets_ ["EST"] = -5;
	TimezoneOffsets_ ["EDT"] = -4;
	TimezoneOffsets_ ["CST"] = -6;
	TimezoneOffsets_ ["CDT"] = -5;
	TimezoneOffsets_ ["MST"] = -7;
	TimezoneOffsets_ ["MDT"] = -6;
	TimezoneOffsets_ ["PST"] = -8;
	TimezoneOffsets_ ["PDT"] = -7;
	TimezoneOffsets_ ["A"] = -1;
	TimezoneOffsets_ ["M"] = -12;
	TimezoneOffsets_ ["N"] = 1;
	TimezoneOffsets_ ["Y"] = +12;
}

RSSParser::~RSSParser ()
{
}

channels_container_t RSSParser::Parse (const channels_container_t& channels,
		channels_container_t& modified,
		const QDomDocument& recent) const
{
	channels_container_t newes = Parse (recent),
        result;
    for (size_t i = 0; i < newes.size (); ++i)
    {
        Channel_ptr newChannel = newes [i];
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
            Channel_ptr pointer = channels [position];
            pointer->Items_ = newChannel->Items_;
            result.push_back (pointer);
        }
        else
        {
            Channel_ptr oldChannel = channels [position];
            Channel_ptr toInsert (new Channel ());
			Channel_ptr modifiedContainer (new Channel ());
			toInsert->Equalify (*oldChannel);
			toInsert->LastBuild_ = newChannel->LastBuild_;
			modifiedContainer->Equalify (*oldChannel);

            for (size_t j = 0; j < newChannel->Items_.size (); ++j)
            {
				items_container_t::const_iterator place =
					std::find_if (oldChannel->Items_.begin (),
							oldChannel->Items_.end (),
							ItemComparator (newChannel->Items_ [j]));

                if (place == oldChannel->Items_.end ())
					toInsert->Items_.push_back (newChannel->Items_ [j]);
				else
					modifiedContainer->Items_.push_back (newChannel->Items_ [j]);
            }
            result.push_back (toInsert);
			modified.push_back (modifiedContainer);
        }
    }
    return result;
}

QDateTime RSSParser::RFC822TimeToQDateTime (const QString& t) const
{
	if (t.size () < 20)
		return QDateTime ();
	
	QString time = t.simplified ();
	short int hoursShift = 0, minutesShift = 0;
	
	QStringList tmp = time.split (' ');
	if (tmp.isEmpty ())
		return QDateTime ();
	if (tmp. at (0).contains (QRegExp ("\\D")))
		tmp.removeFirst ();
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
		hoursShift = TimezoneOffsets_.value (timezone, 0);

 	//HACK: This we don't need this according to rfc, but we added it
 	//	to be compatible with some buggy rss generators
	if (tmp.at (0).size () == 1)
		tmp[0].prepend ("0");
 	tmp [1].truncate (3);
 	//HACK

	time = tmp.join (" ");

	QDateTime result;
	if (tmp.at (2).size () == 4)
		result = QLocale::c ().toDateTime(time, "dd MMM yyyy hh:mm:ss");
	else
		result = QLocale::c ().toDateTime(time, "dd MMM yy hh:mm:ss");
	if (result.isNull () || !result.isValid ())
		return QDateTime ();
	result = result.addSecs (hoursShift * 3600 * (-1) + minutesShift *60 * (-1));
	result.setTimeSpec (Qt::UTC);
	return result.toLocalTime ();
}

