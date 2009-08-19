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

#include "xbelparser.h"
#include <stdexcept>
#include <QObject>
#include <QDomDocument>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			XbelParser::XbelParser (const QByteArray& data)
			{
				QDomDocument document;
				QString errorString;
				int errorLine, errorColumn;
				if (!document.setContent (data, true,
							&errorString, &errorLine, &errorColumn))
					throw std::runtime_error (qPrintable (QObject::tr ("XML parse "
									"error<blockquote>%1</blockquote>at %2:%3.")
								.arg (errorString)
								.arg (errorLine)
								.arg (errorColumn)));
			
				QDomElement root = document.documentElement ();
				if (root.tagName () != "xbel")
					throw std::runtime_error (qPrintable (QObject::tr ("Not an XBEL entity.")));
				else if (root.hasAttribute ("version") &&
						root.attribute ("version") != "1.0")
					throw std::runtime_error (qPrintable (QObject::tr ("This XBEL is not 1.0.")));
			
				QDomElement child = root.firstChildElement ("folder");
				while (!child.isNull ())
				{
					ParseFolder (child);
					child = child.nextSiblingElement ("folder");
				}
			}
			
			void XbelParser::ParseFolder (const QDomElement& element, QStringList previous)
			{
				QString tag = element.firstChildElement ("title").text ();
				if (!tag.isEmpty () && !previous.contains (tag))
					previous << tag;
			
				QDomElement child = element.firstChildElement ();
				while (!child.isNull ())
				{
					if (child.tagName () == "folder")
						ParseFolder (child, previous);
					else if (child.tagName () == "bookmark")
						Core::Instance ().GetFavoritesModel ()->
							AddItem (child.firstChildElement ("title").text (),
									child.attribute ("href"),
									previous);
			
					child = child.nextSiblingElement ();
				}
			}
		};
	};
};

