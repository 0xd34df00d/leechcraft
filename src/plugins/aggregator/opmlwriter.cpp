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

#include "opmlwriter.h"
#include <boost/function.hpp>
#include <QByteArray>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QtDebug>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			OPMLWriter::OPMLWriter ()
			{
			}
			
			OPMLWriter::~OPMLWriter ()
			{
			}
			
			QString OPMLWriter::Write (const channels_shorts_t& channels,
					const QString& title,
					const QString& owner,
					const QString& ownerEmail) const
			{
				QDomDocument doc;
				QDomElement root = doc.createElement ("opml");
				doc.appendChild (root);
				WriteHead (root, doc, title, owner, ownerEmail);
				WriteBody (root, doc, channels);
			
				return doc.toString ();
			}
			
			void OPMLWriter::WriteHead (QDomElement& root,
					QDomDocument& doc,
					const QString& title,
					const QString& owner,
					const QString& ownerEmail) const
			{
				QDomElement head = doc.createElement ("head");
				QDomElement text = doc.createElement ("text");
				head.appendChild (text);
				root.appendChild (head);
			
				if (!title.isEmpty ())
				{
					QDomText t = doc.createTextNode (title);
					text.appendChild (t);
				}
				if (!owner.isEmpty ())
				{
					QDomElement elem = doc.createElement ("owner");
					QDomText t = doc.createTextNode (owner);
					elem.appendChild (t);
					head.appendChild (elem);
				}
				if (!ownerEmail.isEmpty ())
				{
					QDomElement elem = doc.createElement ("ownerEmail");
					QDomText t = doc.createTextNode (ownerEmail);
					elem.appendChild (t);
					head.appendChild (elem);
				}
			}
			
			QString TagGetter (const QDomElement& elem)
			{
				return elem.attribute ("text");
			}
			
			void TagSetter (QDomElement& result, const QString& tag)
			{
				result.setAttribute ("text", tag);
				result.setAttribute ("isOpen", "true");
			}
			
			void OPMLWriter::WriteBody (QDomElement& root,
					QDomDocument& doc,
					const channels_shorts_t& channels) const
			{
				QDomElement body = doc.createElement ("body");
				for (channels_shorts_t::const_iterator i = channels.begin (),
						end = channels.end (); i != end; ++i)
				{
					QStringList tags = i->Tags_;
					tags.sort ();
			
					QDomElement inserter;
					inserter = LeechCraft::Util::GetElementForTags (tags,
							body, doc, "outline",
							boost::function<QString (const QDomElement&)> (TagGetter),
							boost::function<void (QDomElement&, const QString&)> (TagSetter));
					QDomElement item = doc.createElement ("outline");
					item.setAttribute ("title", i->Title_);
					item.setAttribute ("xmlUrl", i->ParentURL_);
					item.setAttribute ("htmlUrl", i->Link_);
					inserter.appendChild (item);
				}
			
				root.appendChild (body);
			}
		};
	};
};

