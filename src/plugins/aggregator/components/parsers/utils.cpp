/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utils.h"
#include <QDomElement>
#include <QStringList>
#include <QtDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include "item.h"
#include "mediarss.h"

namespace LC::Aggregator::Parsers
{
	Item_ptr ParseCommonItem (const QDomElement& entry, IDType_t channelId)
	{
		auto item = std::make_shared<Item> (Item::CreateForChannel (channelId));
		item->Unread_ = true;

		item->Categories_ = GetAllCategories (entry);

		item->Author_ = GetAuthor (entry);
		item->NumComments_ = GetNumComments (entry);
		item->CommentsLink_ = GetCommentsRSS (entry);
		item->CommentsPageLink_ = GetCommentsLink (entry);

		item->Enclosures_ = Atom::GetEnclosures (entry, item->ItemID_) +
				GetEncEnclosures (entry, item->ItemID_);

		item->MRSSEntries_ = MediaRSS::Parse (entry, item->ItemID_);

		const auto& [lat, lon] = GetGeoPoint (entry);
		item->Latitude_ = lat;
		item->Longitude_ = lon;

		return item;
	}

	namespace NS
	{
		const QString DC = "http://purl.org/dc/elements/1.1/";
		const QString WFW = "http://wellformedweb.org/CommentAPI/";
		const QString Atom = "http://www.w3.org/2005/Atom";
		const QString RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
		const QString Slash = "http://purl.org/rss/1.0/modules/slash/";
		const QString Enc = "http://purl.oclc.org/net/rss_2.0/enc#";
		const QString ITunes = "http://www.itunes.com/dtds/podcast-1.0.dtd";
		const QString GeoRSSSimple = "http://www.georss.org/georss";
		const QString GeoRSSW3 = "http://www.w3.org/2003/01/geo/wgs84_pos#";
		const QString Content = "http://purl.org/rss/1.0/modules/content/";
	}

	QString GetLink (const QDomElement& parent)
	{
		for (const auto& link : Util::DomChildren (parent, "link"))
		{
			if (link.attribute ("rel", "alternate") != "alternate")
				continue;

			return link.hasAttribute ("href") ?
					link.attribute ("href") :
					link.text ();
		}
		return {};
	}

	namespace
	{
		QVector<QDomElement> ToList (const QDomNodeList& nodeList)
		{
			QVector<QDomElement> elems;

			const auto size = nodeList.size ();
			elems.reserve (size);
			for (int i = 0; i < size; ++i)
				elems << nodeList.at (i).toElement ();
			return elems;
		}
	}

	QDomElement GetBestDescription (const QDomElement& parent, const QStringList& additionalChildren)
	{
		auto elems = ToList (parent.elementsByTagNameNS (NS::Content, "encoded")) +
				ToList (parent.elementsByTagNameNS (NS::ITunes, "summary"));
		for (const auto& name : additionalChildren)
			if (const auto el = parent.firstChildElement (name);
				!el.isNull ())
				elems << el;

		if (elems.isEmpty ())
			return {};

		return *std::max_element (elems.begin (), elems.end (),
				Util::ComparingBy ([] (const QDomElement& el) { return el.text ().size (); }));
	}

	namespace
	{
		QStringList GetCategoriesFrom (const QDomNodeList& nodes)
		{
			const auto size = nodes.size ();
			QStringList result;
			result.reserve (size);
			for (int i = 0; i < size; ++i)
				if (const auto text = nodes.at (i).toElement ().text ();
					!text.isEmpty ())
					result += nodes.at (i).toElement ().text ();
			return result;
		}
	}

	QStringList GetDCCategories (const QDomElement& parent)
	{
		return GetCategoriesFrom (parent.elementsByTagNameNS (NS::DC, "subject"));
	}

	QStringList GetITunesCategories (const QDomElement& parent)
	{
		return GetCategoriesFrom (parent.elementsByTagNameNS (NS::ITunes, "keywords"));
	}

	QStringList GetPlainCategories (const QDomElement& parent)
	{
		return GetCategoriesFrom (parent.elementsByTagName ("category"));
	}

	QStringList GetAllCategories (const QDomElement& parent)
	{
		return GetDCCategories (parent) +
			GetPlainCategories (parent) +
			GetITunesCategories (parent);
	}

	QString GetAuthor (const QDomElement& parent)
	{
		QString author;
		auto tryField = [&author] (const QDomNodeList& nodes)
		{
			if (!nodes.size ())
				return false;
			author = nodes.at (0).toElement ().text ();
			return true;
		};

		tryField (parent.elementsByTagNameNS (NS::ITunes, "author")) ||
			tryField (parent.elementsByTagNameNS (NS::DC, "creator")) ||
			tryField (parent.elementsByTagName ("author"));

		return author;
	}

	std::optional<QString> GetFirstNodeText (const QDomElement& parent,
			const QString& ns,
			const QString& nodeName)
	{
		const auto nodes = parent.elementsByTagNameNS (ns, nodeName);
		if (!nodes.size ())
			return {};

		return nodes.at (0).toElement ().text ();
	}

	int GetNumComments (const QDomElement& parent)
	{
		const auto& str = GetFirstNodeText (parent, NS::Slash, "comments");
		if (!str)
			return -1;

		bool ok = false;
		const auto res = str->toInt (&ok);
		return ok ? res : -1;
	}

	QString GetCommentsRSS (const QDomElement& parent)
	{
		return GetFirstNodeText (parent, NS::WFW, "commentRss").value_or (QString {});
	}

	QString GetCommentsLink (const QDomElement& parent)
	{
		return GetFirstNodeText (parent, {}, "comments").value_or (QString {});
	}

	QPair<double, double> GetGeoPoint (const QDomElement& parent)
	{
		const auto& latStr = GetFirstNodeText (parent, NS::GeoRSSW3, "lat");
		const auto& longStr = GetFirstNodeText (parent, NS::GeoRSSW3, "long");
		if (latStr && longStr)
			return QPair { latStr->toDouble (), longStr->toDouble () };

		if (const auto& pointStr = GetFirstNodeText (parent, NS::GeoRSSSimple, "point"))
		{
			const auto& splitted = pointStr->splitRef (' ');
			if (splitted.size () == 2)
				return { splitted.at (0).toDouble (), splitted.at (1).toDouble () };
		}

		return {};
	}

	QDateTime GetDCDateTime (const QDomElement& parent)
	{
		if (const auto& dateElem = GetFirstNodeText (parent, NS::DC, "date"))
			return QDateTime::fromString (*dateElem, Qt::ISODate);
		return {};
	}

	QString GetITunesDuration (const QDomElement& parent)
	{
		return GetFirstNodeText (parent, NS::ITunes, "duration").value_or (QString {});
	}

	QList<Enclosure> GetEncEnclosures (const QDomElement& entry, IDType_t itemId)
	{
		const auto& nodes = entry.elementsByTagNameNS (NS::Enc, "enclosure");
		const auto size = nodes.size ();

		QList<Enclosure> result;
		result.reserve (size);

		for (int i = 0; i < nodes.size (); ++i)
		{
			const auto& link = nodes.at (i).toElement ();

			auto e = Enclosure::CreateForItem (itemId);
			e.URL_ = link.attributeNS (NS::RDF, "resource");
			e.Type_ = link.attributeNS (NS::Enc, "type");
			e.Length_ = link.attributeNS (NS::Enc, "length", "-1").toLongLong ();

			result << e;
		}

		return result;
	}

	QString UnescapeHTML (QString&& str)
	{
		return str
				.replace ("&euro;", "€")
				.replace ("&quot;", "\"")
				.replace ("&amp;", "&")
				.replace ("&nbsp;", " ")
				.replace ("&lt;", "<")
				.replace ("&gt;", ">")
				.replace ("&#8217;", "'")
				.replace ("&#8230;", "...");
	}
}

namespace LC::Aggregator::Parsers::Atom
{
	QString ParseEscapeAware (const QDomElement& parent)
	{
		if (parent.attribute ("type", "text") == "text" ||
			parent.attribute ("mode") != "escaped")
			return parent.text ();

		return UnescapeHTML (parent.text ());
	}

	QList<Enclosure> GetEnclosures (const QDomElement& entry, IDType_t itemId)
	{
		const auto& links = entry.elementsByTagName ("link");
		const auto size = links.size ();

		QList<Enclosure> result;
		result.reserve (size);
		for (int i = 0; i < size; ++i)
		{
			const auto& link = links.at (i).toElement ();
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

namespace LC::Aggregator::Parsers::RSS
{
	QList<Enclosure> GetEnclosures (const QDomElement& entry, IDType_t itemId)
	{
		const auto& links = entry.elementsByTagName ("enclosure");
		const auto size = links.size ();

		QList<Enclosure> result;
		result.reserve (size);
		for (int i = 0; i < size; ++i)
		{
			const auto& link = links.at (i).toElement ();

			auto e = Enclosure::CreateForItem (itemId);
			e.URL_ = link.attribute ("url");
			e.Type_ = link.attribute ("type");
			e.Length_ = link.attribute ("length", "-1").toLongLong ();
			e.Lang_ = link.attribute ("hreflang");
			result << e;
		}
		return result;
	}
}
