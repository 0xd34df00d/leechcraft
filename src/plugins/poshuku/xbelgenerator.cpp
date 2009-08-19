/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <plugininterface/util.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
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
				for (FavoritesModel::items_t::const_iterator i =
						Core::Instance ().GetFavoritesModel ()->GetItems ().begin (),
						end = Core::Instance ().GetFavoritesModel ()->GetItems ().end ();
						i != end; ++i)
				{
					QDomElement inserter = LeechCraft::Util::GetElementForTags (i->Tags_,
							root, document, "folder",
							boost::function<QString (const QDomElement&)> (TagGetter),
							boost::bind (TagSetter, document, _1, _2));
			
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
		};
	};
};

