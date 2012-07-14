/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "xbelgenerator.h"
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <util/util.h>
#include "core.h"

namespace LeechCraft
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
