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

#ifndef PLUGINS_AGGREGATOR_PARSER_H
#define PLUGINS_AGGREGATOR_PARSER_H
#include <QDomDocument>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "channel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class Parser
			{
			public:
				virtual ~Parser ();
				/** @brief Indicates whether parser could parse the document.
				 *
				 * @param[in] doc 
				 */
				virtual bool CouldParse (const QDomDocument& doc) const = 0;

				/** @brief Parses the document
				 * 
				 * Parses the passed XML document, differs it with already existing
				 * elements passed in old, puts new elements into returned container
				 * and already existing ones - into modified parameter.
				 *
				 * @param[in] old Already existing items.
				 * @param[out] modified Modified items.
				 * @param[in] document Byte array with XML document.
				 * @return Container (channels_container_t) with new items.
				 */
				virtual channels_container_t Parse (const channels_container_t& old,
						channels_container_t& modified,
						const QDomDocument& document) const;
			protected:
				static const QString DC_;
				static const QString WFW_;
				static const QString Atom_;
				static const QString RDF_;
				static const QString Slash_;
				static const QString Enc_;
				static const QString ITunes_;

				virtual channels_container_t Parse (const QDomDocument&) const = 0;
				QString GetLink (const QDomElement&) const;
				QString GetAuthor (const QDomElement&) const;
				QString GetCommentsRSS (const QDomElement&) const;
				QString GetCommentsLink (const QDomElement&) const;
				int GetNumComments (const QDomElement&) const;
				QDateTime GetDCDateTime (const QDomElement&) const;
				QStringList GetAllCategories (const QDomElement&) const;
				QStringList GetDCCategories (const QDomElement&) const;
				QStringList GetITunesCategories (const QDomElement&) const;
				QStringList GetPlainCategories (const QDomElement&) const;
				QList<Enclosure> GetEncEnclosures (const QDomElement&) const;

				QDateTime FromRFC3339 (const QString&) const;
				QString UnescapeHTML (const QString&) const;
			};
		};
	};
};

#endif

