/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QMetaType>
#include <QStringList>
#include <QDateTime>
#include <QHash>
#include <QUrl>
#include <QByteArrayMatcher>
#include <util/sll/regexp.h>

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	struct FilterOption
	{
		Qt::CaseSensitivity Case_ = Qt::CaseInsensitive;

		enum class MatchType
		{
			Wildcard,
			Regexp,
			Plain,
			Begin,
			End
		} MatchType_ = MatchType::Wildcard;

		enum MatchObject
		{
			All = 0x00,
			Script = 0x01,
			Image = 0x02,
			Object = 0x04,
			CSS = 0x08,
			ObjSubrequest = 0x10,
			Subdocument = 0x20,
			AJAX = 0x40,
			Popup = 0x80
		};
		Q_DECLARE_FLAGS (MatchObjects, MatchObject)
		MatchObjects MatchObjects_;

		QStringList Domains_;
		QStringList NotDomains_;
		QString HideSelector_;

		enum class ThirdParty
		{
			Yes,
			No,
			Unspecified
		} ThirdParty_ = ThirdParty::Unspecified;
	};

	QDataStream& operator<< (QDataStream&, const FilterOption&);
	QDataStream& operator>> (QDataStream&, FilterOption&);

	bool operator== (const FilterOption&, const FilterOption&);
	bool operator!= (const FilterOption&, const FilterOption&);

	QDebug operator<< (QDebug, const FilterOption&);

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
		Util::RegExp RegExp_;
		QByteArray PlainMatcher_;
		FilterOption Option_;
	};

	QDebug operator<< (QDebug, const FilterItem&);

	typedef std::shared_ptr<FilterItem> FilterItem_ptr;

	QDataStream& operator<< (QDataStream&, const FilterItem&);
	QDataStream& operator>> (QDataStream&, FilterItem&);

	struct Filter
	{
		QList<FilterItem_ptr> Filters_;
		QList<FilterItem_ptr> Exceptions_;

		SubscriptionData SD_;

		Filter& operator+= (const Filter&);
	};
}
}
}

Q_DECLARE_METATYPE (LC::Poshuku::CleanWeb::FilterItem)
Q_DECLARE_METATYPE (QList<LC::Poshuku::CleanWeb::FilterItem>)
