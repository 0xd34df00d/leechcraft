#include "atomparser.h"
#include <QDomDocument>
#include <QString>
#include <QtDebug>

channels_container_t AtomParser::Parse (const channels_container_t& old,
		const QDomDocument& recent) const
{
	channels_container_t newes = Parse (recent),
        result;
    if (!newes.size ())
        return channels_container_t ();
    else if (!old.size ())
        return newes;
    else
    {
        Channel_ptr toInsert (new Channel ());
        toInsert->Equalify (*old [0]);
        Item_ptr lastItemWeHave = old [0]->Items_ [0];
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

QDateTime AtomParser::FromRFC3339 (const QString& t) const
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

QString AtomParser::GetLink (const QDomElement& parent) const
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

QString AtomParser::ParseEscapeAware (const QDomElement& parent) const
{
	QString result;
    if (!parent.hasAttribute ("type") ||
			parent.attribute ("type") == "text" ||
			(parent.attribute ("type") == "text/html" &&
			 parent.attribute ("mode") != "escaped"))
        result = parent.text ();
    else if (parent.attribute ("type") == "text/html" &&
			parent.attribute ("mode") == "escaped")
        result = UnescapeHTML (parent.text ());
	else
		result = UnescapeHTML (parent.text ());

	return result;
}

