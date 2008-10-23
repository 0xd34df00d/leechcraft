#include "atomparser.h"
#include <QDomDocument>
#include <QString>
#include <QtDebug>

channels_container_t AtomParser::Parse (const channels_container_t& old,
		channels_container_t& modified,
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
		Channel_ptr modifiedContainer (new Channel ());
        toInsert->Equalify (*old [0]);
		modifiedContainer->Equalify (*old [0]);
        Item_ptr lastItemWeHave;
		if (old [0]->Items_.size ())
			lastItemWeHave = old [0]->Items_ [0];
		else
			lastItemWeHave.reset (new Item);

		items_container_t::iterator itemPosition =
			std::find_if (newes [0]->Items_.begin (), newes [0]->Items_.end (),
					ItemComparator (lastItemWeHave));

		toInsert->Items_.insert (toInsert->Items_.end (),
				newes [0]->Items_.begin (), itemPosition);

		if (itemPosition != newes [0]->Items_.end ())
			modifiedContainer->Items_.insert (modifiedContainer->Items_.end (),
					itemPosition + 1, newes [0]->Items_.end ());

        result.push_back (toInsert);
		modified.push_back (modifiedContainer);
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

