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
#include <QtDebug>
#include <util/sll/debugprinters.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>

namespace LC::Aggregator
{
	namespace
	{
		OPMLInfo ParseInfo (const QDomDocument& doc)
		{
			OPMLInfo result;
			const auto head = doc.documentElement ().firstChildElement ("head"_qs);
			for (const auto& elem : Util::DomChildren (head, {}))
				result [elem.tagName ()] = elem.text ();
			return result;
		}

		void ParseOutline (const QDomElement& outline, QStringList categories, QList<OPMLItem>& result)
		{
			if (outline.attribute ("text"_qs).size ())
				categories << outline.attribute ("text"_qs);

			if (outline.hasAttribute ("xmlUrl"_qs))
			{
				OPMLItem item;
				item.URL_ = outline.attribute ("xmlUrl"_qs);
				item.HTMLUrl_ = outline.attribute ("htmlUrl"_qs);
				item.Title_ = outline.attribute ("title"_qs);
				item.CustomFetchInterval_ = outline.attribute ("useCustomFetchInterval"_qs) == "true"_ql;
				item.MaxArticleAge_ = outline.attribute ("maxArticleAge"_qs).toInt ();
				item.FetchInterval_ = outline.attribute ("fetchInterval"_qs).toInt ();
				item.MaxArticleNumber_ = outline.attribute ("maxArticleNumber"_qs).toInt ();
				item.Description_ = outline.attribute ("description"_qs);
				item.Categories_ = categories;

				result.push_back (item);
			}

			for (const auto& childOutline : Util::DomChildren (outline, "outline"_qs))
				ParseOutline (childOutline, categories, result);
		}

		QList<OPMLItem> ParseOutlines (const QDomDocument& doc)
		{
			QList<OPMLItem> result;
			const auto& body = doc.documentElement ().firstChildElement ("body"_qs);
			for (const auto& outline : Util::DomChildren (body, "outline"_qs))
				ParseOutline (outline, {}, result);
			return result;
		}
	}

	Util::Either<QString, OPMLParseResult> ParseOPML (const QString& filename)
	{
		QFile file { filename };
		if (!file.open (QIODevice::ReadOnly))
			return Util::Left { QObject::tr ("could not open file %1 for reading").arg (filename) };

		QDomDocument document;
		if (const auto res = document.setContent (&file, QDomDocument::ParseOption::UseNamespaceProcessing);
			!res)
		{
			qWarning () << "unable to parse" << filename << res;
			return Util::Left { QObject::tr ("malformed XML file").arg (filename) };
		}

		return OPMLParseResult { .Info_ = ParseInfo (document), .Items_ = ParseOutlines (document) };
	}
}
