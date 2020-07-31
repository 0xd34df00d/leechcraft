/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xbelgenerator.h"
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <util/util.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
	QString TagGetter (const QDomElement& elem)
	{
		if (elem.tagName () == "folder")
			return elem.firstChildElement ("title").text ();
		else
			return QString ();
	}

	void TagSetter (QDomDocument& doc,
			QDomElement& elem, const QString& tag)
	{
		QDomElement title = doc.createElement ("title");
		QDomText text = doc.createTextNode (tag);
		title.appendChild (text);
		elem.appendChild (title);
	}

	XbelGenerator::XbelGenerator (QByteArray& output)
	{
		QDomDocument document;
		QDomElement root = document.createElement ("xbel");
		root.setAttribute ("version", "1.0");
		document.appendChild (root);

		auto items = Core::Instance ().GetFavoritesModel ()->GetItems ();
		for (auto i = items.begin (), end = items.end ();
				i != end; ++i)
		{
			QDomElement inserter = Util::GetElementForTags (i->Tags_,
					root, document, "folder", TagGetter,
					[&document] (QDomElement& elem, const QString& tag) { TagSetter (document, elem, tag); });

			QDomElement item = document.createElement ("bookmark");
			item.setAttribute ("href", i->URL_);

			QDomElement title = document.createElement ("title");
			QDomText titleText = document.createTextNode (i->Title_);
			title.appendChild (titleText);
			item.appendChild (title);

			inserter.appendChild (item);
		}

		output = document.toByteArray (4);
	}
}
}
