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

#include "description.h"
#include <stdexcept>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			QDataStream& operator<< (QDataStream& out, const UrlDescription& d)
			{
				quint8 version = 1;
				out << version
					<< d.Template_
					<< d.Type_
					<< d.IndexOffset_
					<< d.PageOffset_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, UrlDescription& d)
			{
				quint8 version = 0;
				in >> version;
				if (version != 1)
					throw std::runtime_error ("Unknown version for UrlDescription");
				in >> d.Template_
					>> d.Type_
					>> d.IndexOffset_
					>> d.PageOffset_;
				return in;
			}

			QUrl UrlDescription::MakeUrl (const QString& searchStr, const QHash<QString, QVariant>& params) const
			{
				QUrl url (Template_);
				QList<QPair<QString, QString>> items = url.queryItems (),
					newItems;
				QPair<QString, QString> item;
				Q_FOREACH (item, items)
				{
					// Currently skips optional parameters
					if (item.second.size () >= 3 &&
							item.second.at (0) == '{' &&
							item.second.at (item.second.size () - 1) == '}' &&
							item.second.at (item.second.size () - 2) == '?')
						continue;

					if (item.second == "{searchTerms}")
						item.second = searchStr;
					else if (item.second.size () > 2 &&
							*item.second.begin () == '{' &&
							*(item.second.end () - 1) == '}')
					{
						QString key = item.second.mid (1,
								item.second.size () - 2);
						// To the correct string if Params_ has this key or to
						// empty string otherwise.
						item.second = params [key].toString ();
					}
					else
						item.second = "";

					newItems << item;
				}
				url.setQueryItems (newItems);
				return url;
			}

			QDataStream& operator<< (QDataStream& out, const QueryDescription& d)
			{
				quint8 version = 1;
				out << version
					<< static_cast<quint8> (d.Role_)
					<< d.Title_
					<< d.TotalResults_
					<< d.SearchTerms_
					<< d.Count_
					<< d.StartIndex_
					<< d.StartPage_
					<< d.Language_
					<< d.InputEncoding_
					<< d.OutputEncoding_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, QueryDescription& d)
			{
				quint8 version = 0;
				in >> version;
				if (version != 1)
					throw std::runtime_error ("Unknown version for QueryDescription");
				quint8 role;
				in >> role;
				d.Role_ = static_cast<QueryDescription::Role> (role);
				in >> d.Title_
					>> d.TotalResults_
					>> d.SearchTerms_
					>> d.Count_
					>> d.StartIndex_
					>> d.StartPage_
					>> d.Language_
					>> d.InputEncoding_
					>> d.OutputEncoding_;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const Description& d)
			{
				quint8 version = 1;
				out << version
					<< d.ShortName_
					<< d.Description_
					<< d.URLs_
					<< d.Contact_
					<< d.Tags_
					<< d.Queries_
					<< d.Developer_
					<< d.Attribution_
					<< static_cast<quint8> (d.Right_)
					<< d.Adult_
					<< d.Languages_
					<< d.InputEncodings_
					<< d.OutputEncodings_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, Description& d)
			{
				quint8 version = 0;
				in >> version;
				if (version != 1)
					throw std::runtime_error ("Unknown version for Description");
				in >> d.ShortName_
					>> d.Description_
					>> d.URLs_
					>> d.Contact_
					>> d.Tags_
					>> d.Queries_
					>> d.Developer_
					>> d.Attribution_;
				quint8 sr;
				in >> sr;
				d.Right_ = static_cast<Description::SyndicationRight> (sr);
				in >> d.Adult_
					>> d.Languages_
					>> d.InputEncodings_
					>> d.OutputEncodings_;
				return in;
			}

		};
	};
};

