/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_OPMLPARSER_H
#define PLUGINS_AGGREGATOR_OPMLPARSER_H
#include <vector>
#include <QHash>
#include <QString>
#include <QDomDocument>
#include "opmlitem.h"

class QDomElement;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class OPMLParser
			{
			public:
				typedef std::vector<OPMLItem> items_container_t;
				typedef QHash<QString, QString> OPMLinfo_t;
			private:
				mutable items_container_t Items_;
				mutable bool CacheValid_;
				QDomDocument Document_;
			public:
				OPMLParser (const QDomDocument&);

				void Reset (const QDomDocument&);
				bool IsValid () const;
				OPMLinfo_t GetInfo () const;
				items_container_t Parse () const;
			private:
				void ParseOutline (const QDomElement&,
						QStringList = QStringList ()) const;
			};
		};
	};
};

#endif

