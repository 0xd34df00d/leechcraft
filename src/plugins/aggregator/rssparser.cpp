/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rssparser.h"
#include <QDomDocument>
#include <QLocale>
#include <QtDebug>

namespace LC
{
namespace Aggregator
{
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
	
	QList<Enclosure> RSSParser::GetEnclosures (const QDomElement& entry, const IDType_t& item) const
	{
		QList<Enclosure> result;
		QDomNodeList links = entry.elementsByTagName ("enclosure");
		for (int i = 0; i < links.size (); ++i)
		{
			QDomElement link = links.at (i).toElement ();

			auto e = Enclosure::CreateForItem (item);
			e.URL_ = link.attribute ("url");
			e.Type_ = link.attribute ("type");
			e.Length_ = link.attribute ("length", "-1").toLongLong ();
			e.Lang_ = link.attribute ("hreflang");
			result << e;
		}
		return result;
	}
}
}
