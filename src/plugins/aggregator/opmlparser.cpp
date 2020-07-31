/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "opmlparser.h"
#include <QFile>
#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <util/sll/domchildrenrange.h>
#include <util/sll/either.h>
#include <util/sll/functor.h>

namespace LC
{
namespace Aggregator
{
	OPMLParser::OPMLParser (const QDomDocument& document)
	: Document_ (document)
	{
	}
	
	bool OPMLParser::IsValid ()
	{
		auto root = Document_.documentElement ();
		if (root.tagName () != "opml")
			return false;
	
		auto heads = root.elementsByTagName ("head");
		if (heads.size () != 1 || !heads.at (0).isElement ())
			return false;
	
		auto bodies = root.elementsByTagName ("body");
		if (bodies.size () != 1 || !bodies.at (0).isElement ())
			return false;

		return !bodies.at (0).toElement ().elementsByTagName ("outline").isEmpty ();

	}
	
	OPMLParser::OPMLinfo_t OPMLParser::GetInfo ()
	{
		OPMLinfo_t result;
	
		auto entries = Document_.documentElement ().firstChildElement ("head").childNodes ();
	
		for (int i = 0; i < entries.size (); ++i)
		{
			auto elem = entries.at (i).toElement ();
			if (elem.isNull ())
				continue;
	
			result [elem.tagName ()] = elem.text ();
		}
	
		return result;
	}
	
	OPMLParser::items_container_t OPMLParser::Parse ()
	{
		if (!CacheValid_)
		{
			Items_.clear ();

			auto body = Document_.documentElement ().firstChildElement ("body");
			for (const auto& outline : Util::DomChildren (body, "outline"))
				ParseOutline (outline);

			CacheValid_ = true;
		}
	
		return Items_;
	}
	
	void OPMLParser::ParseOutline (const QDomElement& parentOutline, QStringList previousStrings)
	{
		if (parentOutline.hasAttribute ("xmlUrl"))
		{
			OPMLItem item;
			item.URL_ = parentOutline.attribute ("xmlUrl");
			item.HTMLUrl_ = parentOutline.attribute ("htmlUrl");
			item.Title_ = parentOutline.attribute ("title");
			item.CustomFetchInterval_ = parentOutline.attribute ("useCustomFetchInterval") == "true";
			item.MaxArticleAge_ = parentOutline.attribute ("maxArticleAge").toInt ();
			item.FetchInterval_ = parentOutline.attribute ("fetchInterval").toInt ();
			item.MaxArticleNumber_ = parentOutline.attribute ("maxArticleNumber").toInt ();
			item.Description_ = parentOutline.attribute ("description");
			item.Categories_ = previousStrings;
	
			Items_.push_back (item);
		}
	
		if (parentOutline.attribute ("text").size ())
			previousStrings << parentOutline.attribute ("text");

		for (const auto& outline : Util::DomChildren (parentOutline, "outline"))
			ParseOutline (outline, previousStrings);
	}

	OPMLParseResult_t ParseOPML (const QString& filename)
	{
		QFile file { filename };
		if (!file.open (QIODevice::ReadOnly))
			return OPMLParseResult_t::Left (QObject::tr ("Could not open file %1 for reading.")
					.arg (filename));

		QString errorMsg;
		int errorLine, errorColumn;
		QDomDocument document;
		if (!document.setContent (&file,
				true,
				&errorMsg,
				&errorLine,
				&errorColumn))
			return OPMLParseResult_t::Left (QObject::tr ("XML error, file %1, line %2, column %3, error:<br />%4")
					.arg (filename)
					.arg (errorLine)
					.arg (errorColumn)
					.arg (errorMsg));

		OPMLParser parser { document };
		if (!parser.IsValid ())
			return OPMLParseResult_t::Left (QObject::tr ("OPML from file %1 is not valid.")
					.arg (filename));

		return OPMLParseResult_t::Right (parser);
	}

	OPMLItemsResult_t ParseOPMLItems (const QString& filename)
	{
		return ParseOPML (filename) * [] (auto parser) { return parser.Parse (); };
	}
}
}
