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

#pragma once

#include <QMetaType>
#include <QStringList>
#include <QDateTime>
#include <QHash>
#include <QUrl>

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	struct FilterOption
	{
		Qt::CaseSensitivity Case_;
		enum MatchType
		{
			MTWildcard,
			MTRegexp,
			MTPlain,
			MTBegin,
			MTEnd
		} MatchType_;
		enum MatchObject
		{
			MatchScript = 0x01,
			MatchImage = 0x02,
		};
		QStringList Domains_;
		QStringList NotDomains_;
		QString HideSelector_;
		bool AbortForeign_;

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

	struct FilterItem
	{
		QString OrigString_;
		QRegExp RegExp_;
		QStringMatcher PlainMatcher_;
		FilterOption Option_;
	};

	QDataStream& operator<< (QDataStream&, const FilterItem&);
	QDataStream& operator>> (QDataStream&, FilterItem&);

	struct Filter
	{
		QList<FilterItem> Filters_;
		QList<FilterItem> Exceptions_;

		SubscriptionData SD_;

		Filter& operator+= (const Filter&);
	};
}
}
}

Q_DECLARE_METATYPE (LeechCraft::Poshuku::CleanWeb::FilterItem);
Q_DECLARE_METATYPE (QList<LeechCraft::Poshuku::CleanWeb::FilterItem>);
