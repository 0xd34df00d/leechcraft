/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utils.h"
#include <QDomElement>
#include <QRegularExpression>
#include <QStringList>
#include <QtDebug>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
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
		const QString DC = "http://purl.org/dc/elements/1.1/"_qs;
		const QString WFW = "http://wellformedweb.org/CommentAPI/"_qs;
		const QString Atom = "http://www.w3.org/2005/Atom"_qs;
		const QString RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#"_qs;
		const QString Slash = "http://purl.org/rss/1.0/modules/slash/"_qs;
		const QString Enc = "http://purl.oclc.org/net/rss_2.0/enc#"_qs;
		const QString ITunes = "http://www.itunes.com/dtds/podcast-1.0.dtd"_qs;
		const QString GeoRSSSimple = "http://www.georss.org/georss"_qs;
		const QString GeoRSSW3 = "http://www.w3.org/2003/01/geo/wgs84_pos#"_qs;
		const QString Content = "http://purl.org/rss/1.0/modules/content/"_qs;
	}

	QString GetLink (const QDomElement& parent)
	{
		for (const auto& link : Util::DomChildren (parent, "link"_qs))
		{
			if (link.attribute ("rel"_qs, "alternate"_qs) != "alternate"_ql)
				continue;

			return link.hasAttribute ("href"_qs) ?
					link.attribute ("href"_qs) :
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
		auto elems = ToList (parent.elementsByTagNameNS (NS::Content, "encoded"_qs)) +
				ToList (parent.elementsByTagNameNS (NS::ITunes, "summary"_qs));
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
		return GetCategoriesFrom (parent.elementsByTagNameNS (NS::DC, "subject"_qs));
	}

	QStringList GetITunesCategories (const QDomElement& parent)
	{
		return GetCategoriesFrom (parent.elementsByTagNameNS (NS::ITunes, "keywords"_qs));
	}

	QStringList GetPlainCategories (const QDomElement& parent)
	{
		return GetCategoriesFrom (parent.elementsByTagName ("category"_qs));
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

		tryField (parent.elementsByTagNameNS (NS::ITunes, "author"_qs)) ||
			tryField (parent.elementsByTagNameNS (NS::DC, "creator"_qs)) ||
			tryField (parent.elementsByTagName ("author"_qs));

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
		const auto& str = GetFirstNodeText (parent, NS::Slash, "comments"_qs);
		if (!str)
			return -1;

		bool ok = false;
		const auto res = str->toInt (&ok);
		return ok ? res : -1;
	}

	QString GetCommentsRSS (const QDomElement& parent)
	{
		return GetFirstNodeText (parent, NS::WFW, "commentRss"_qs).value_or (QString {});
	}

	QString GetCommentsLink (const QDomElement& parent)
	{
		return GetFirstNodeText (parent, {}, "comments"_qs).value_or (QString {});
	}

	QPair<double, double> GetGeoPoint (const QDomElement& parent)
	{
		const auto& latStr = GetFirstNodeText (parent, NS::GeoRSSW3, "lat"_qs);
		const auto& longStr = GetFirstNodeText (parent, NS::GeoRSSW3, "long"_qs);
		if (latStr && longStr)
			return QPair { latStr->toDouble (), longStr->toDouble () };

		if (const auto& pointStr = GetFirstNodeText (parent, NS::GeoRSSSimple, "point"_qs))
		{
			const auto& splitted = pointStr->splitRef (' ');
			if (splitted.size () == 2)
				return { splitted.at (0).toDouble (), splitted.at (1).toDouble () };
		}

		return {};
	}

	QDateTime GetDCDateTime (const QDomElement& parent)
	{
		if (const auto& dateElem = GetFirstNodeText (parent, NS::DC, "date"_qs))
			return QDateTime::fromString (*dateElem, Qt::ISODate);
		return {};
	}

	QString GetITunesDuration (const QDomElement& parent)
	{
		return GetFirstNodeText (parent, NS::ITunes, "duration"_qs).value_or (QString {});
	}

	QList<Enclosure> GetEncEnclosures (const QDomElement& entry, IDType_t itemId)
	{
		const auto& nodes = entry.elementsByTagNameNS (NS::Enc, "enclosure"_qs);
		const auto size = nodes.size ();

		QList<Enclosure> result;
		result.reserve (size);

		for (int i = 0; i < nodes.size (); ++i)
		{
			const auto& link = nodes.at (i).toElement ();

			auto e = Enclosure::CreateForItem (itemId);
			e.URL_ = link.attributeNS (NS::RDF, "resource"_qs);
			e.Type_ = link.attributeNS (NS::Enc, "type"_qs);
			e.Length_ = link.attributeNS (NS::Enc, "length"_qs, "-1"_qs).toLongLong ();

			result << e;
		}

		return result;
	}

	namespace
	{
		void ConvertNumberEntities (QString& str)
		{
			if (!str.contains ("&#"_ql))
				return;

			static thread_local const QRegularExpression rx { R"(&#\d+;)" };
			for (auto match = rx.match (str); match.hasMatch (); match = rx.match (str, match.capturedStart (0) + 1))
			{
				const auto& matchingStr = match.capturedView ().mid (2).chopped (1);

				bool ok = false;
				const auto code = matchingStr.toInt (&ok);
				if (!ok)
					continue;

				str.replace (match.capturedStart (), match.capturedLength (), QChar { code });
			}
		}
	}

	QString UnescapeHTML (QString&& str)
	{
		str .replace ("&euro;"_ql, "€"_ql)
			.replace ("&quot;"_ql, "\""_ql)
			.replace ("&amp;"_ql, "&"_ql)
			.replace ("&nbsp;"_ql, " "_ql)
			.replace ("&lt;"_ql, "<"_ql)
			.replace ("&gt;"_ql, ">"_ql)
			.replace ("&#8217;"_qs, "'"_ql)
			.replace ("&#8230;"_qs, u"…"_qs)
			.replace ("&laquo;"_qs, u"«"_qs)
			.replace ("&raquo;"_qs, u"»"_qs)
			.replace ("&ndash;"_qs, "-"_qs)
			.replace ("&mdash;"_qs, u"—"_qs)
			;
		ConvertNumberEntities (str);
		return str;
	}
}

namespace LC::Aggregator::Parsers::Atom
{
	QString ParseEscapeAware (const QDomElement& parent)
	{
		if (parent.attribute ("type"_qs, "text"_qs) == "text"_ql ||
			parent.attribute ("mode"_qs) != "escaped"_ql)
			return parent.text ();

		return UnescapeHTML (parent.text ());
	}

	QList<Enclosure> GetEnclosures (const QDomElement& entry, IDType_t itemId)
	{
		const auto& links = entry.elementsByTagName ("link"_qs);
		const auto size = links.size ();

		QList<Enclosure> result;
		result.reserve (size);
		for (int i = 0; i < size; ++i)
		{
			const auto& link = links.at (i).toElement ();
			if (link.attribute ("rel"_qs) != "enclosure"_ql)
				continue;

			auto e = Enclosure::CreateForItem (itemId);
			e.URL_ = link.attribute ("href"_qs);
			e.Type_ = link.attribute ("type"_qs);
			e.Length_ = link.attribute ("length"_qs, "-1"_qs).toLongLong ();
			e.Lang_ = link.attribute ("hreflang"_qs);
			result << e;
		}
		return result;
	}
}

namespace LC::Aggregator::Parsers::RSS
{
	QList<Enclosure> GetEnclosures (const QDomElement& entry, IDType_t itemId)
	{
		const auto& links = entry.elementsByTagName ("enclosure"_qs);
		const auto size = links.size ();

		QList<Enclosure> result;
		result.reserve (size);
		for (int i = 0; i < size; ++i)
		{
			const auto& link = links.at (i).toElement ();

			auto e = Enclosure::CreateForItem (itemId);
			e.URL_ = link.attribute ("url"_qs);
			e.Type_ = link.attribute ("type"_qs);
			e.Length_ = link.attribute ("length"_qs, "-1"_qs).toLongLong ();
			e.Lang_ = link.attribute ("hreflang"_qs);
			result << e;
		}
		return result;
	}
}
