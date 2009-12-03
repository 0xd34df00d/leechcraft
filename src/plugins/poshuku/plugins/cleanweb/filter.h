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

#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_FILTER_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_FILTER_H
#include <QMetaType>
#include <QStringList>
#include <QDateTime>
#include <QHash>
#include <QUrl>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					struct FilterOption
					{
						Qt::CaseSensitivity Case_;
						enum MatchType
						{
							MTWildcard,
							MTRegexp
						};
						MatchType MatchType_;
						QStringList Domains_;
						QStringList NotDomains_;

						FilterOption ();
					};

					QDataStream& operator<< (QDataStream&, const FilterOption&);
					QDataStream& operator>> (QDataStream&, FilterOption&);

					bool operator== (const FilterOption&, const FilterOption&);
					bool operator!= (const FilterOption&, const FilterOption&);

					struct SubscriptionData
					{
						/// The URL of the subscription.
						QUrl URL_;
						/** The name of the subscription as provided by the abp:
						 * link.
						 */
						QString Name_;
						/// This is the name of the file inside the
						//~/.leechcraft/cleanweb/.
						QString Filename_;
						/// The date/time of last update.
						QDateTime LastDateTime_;
					};

					typedef QHash<QString, FilterOption> OptionsDict_t;
					typedef QHash<QString, QRegExp> RegExpsDict_t;

					struct Filter
					{
						QStringList ExceptionStrings_;
						QStringList FilterStrings_;
						OptionsDict_t Options_;
						RegExpsDict_t RegExps_;

						SubscriptionData SD_;
					};
				};
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::RegExpsDict_t);
Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::OptionsDict_t);

#endif

