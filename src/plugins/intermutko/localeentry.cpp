/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localeentry.h"
#include <QDataStream>
#include <QtDebug>

namespace LC
{
namespace Intermutko
{
	bool operator== (const LocaleEntry& l1, const LocaleEntry& l2)
	{
		return l1.Country_ == l2.Country_ &&
				l1.Language_ == l2.Language_;
	}

	bool operator!= (const LocaleEntry& l1, const LocaleEntry& l2)
	{
		return !(l1 == l2);
	}

	QDataStream& operator<< (QDataStream& out, const LocaleEntry& entry)
	{
		out << static_cast<quint8> (1)
				<< static_cast<qint32> (entry.Language_)
				<< static_cast<qint32> (entry.Country_)
				<< entry.Q_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, LocaleEntry& entry)
	{
		quint8 version = 0;
		in >> version;
		if (version < 1 || version > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		qint32 lang = 0;
		qint32 country = 0;
		in >> lang
				>> country
				>> entry.Q_;
		entry.Language_ = static_cast<QLocale::Language> (lang);
		entry.Country_ = static_cast<QLocale::Country> (country);
		return in;
	}
}
}
