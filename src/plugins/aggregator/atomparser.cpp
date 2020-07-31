/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "atomparser.h"
#include <QDomDocument>
#include <QString>
#include <QtDebug>

namespace LC
{
namespace Aggregator
{
	AtomParser::AtomParser ()
	{
	}

	AtomParser::~AtomParser ()
	{
	}

	QString AtomParser::ParseEscapeAware (const QDomElement& parent) const
	{
		QString result;
		if (!parent.hasAttribute ("type") ||
				parent.attribute ("type") == "text" ||
				(parent.attribute ("type") == "text/html" &&
					parent.attribute ("mode") != "escaped"))
			result = parent.text ();
		else
			result = UnescapeHTML (parent.text ());
	
		return result;
	}
	
	QList<Enclosure> AtomParser::GetEnclosures (const QDomElement& entry,
			const IDType_t& itemId) const
	{
		QList<Enclosure> result;
		QDomNodeList links = entry.elementsByTagName ("link");
		for (int i = 0; i < links.size (); ++i)
		{
			QDomElement link = links.at (i).toElement ();
			if (link.attribute ("rel") != "enclosure")
				continue;

			auto e = Enclosure::CreateForItem (itemId);
			e.URL_ = link.attribute ("href");
			e.Type_ = link.attribute ("type");
			e.Length_ = link.attribute ("length", "-1").toLongLong ();
			e.Lang_ = link.attribute ("hreflang");
			result << e;
		}
		return result;
	}
}
}
