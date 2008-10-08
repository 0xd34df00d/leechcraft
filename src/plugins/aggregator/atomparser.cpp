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

