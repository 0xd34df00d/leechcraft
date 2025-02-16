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

namespace LC::Intermutko
{
	bool operator== (const LocaleEntry& l1, const LocaleEntry& l2)
	{
		return l1.Locale_ == l2.Locale_;
	}

	QDataStream& operator<< (QDataStream& out, const LocaleEntry& entry)
	{
		out << static_cast<quint8> (2)
			<< entry.Locale_
			<< entry.Q_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, LocaleEntry& entry)
	{
		quint8 version = 0;
		in >> version;
		switch (version)
		{
		case 1:
		{
			qint32 lang = 0;
			qint32 country = 0;
			in >> lang
				>> country
				>> entry.Q_;
			entry.Locale_ = QLocale { static_cast<QLocale::Language> (lang), static_cast<QLocale::Country> (country) };
			break;
		}
		case 2:
			in >> entry.Locale_
				>> entry.Q_;
			break;
		default:
			qWarning () << "unknown version" << version;
			break;
		}

		return in;
	}
}
