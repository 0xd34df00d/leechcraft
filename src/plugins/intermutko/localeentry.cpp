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
		return l1.Language_ == l2.Language_ &&
				l1.Territory_ == l2.Territory_;
	}

	QDataStream& operator<< (QDataStream& out, const LocaleEntry& entry)
	{
		out << static_cast<quint8> (3)
			<< static_cast<qint32> (entry.Language_)
			<< static_cast<qint32> (entry.Territory_)
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
		case 3:
		{
			qint32 language = 0;
			qint32 territory = 0;
			in >> language
				>> territory
				>> entry.Q_;
			entry.Language_ = static_cast<QLocale::Language> (language);
			entry.Territory_ = static_cast<QLocale::Territory> (territory);
			break;
		}
		case 2:
		{
			QLocale locale;
			in >> locale
				>> entry.Q_;
			entry.Language_ = locale.language ();
			entry.Territory_ = locale.territory ();
			break;
		}
		default:
			qWarning () << "unknown version" << version;
			in.setStatus (QDataStream::ReadCorruptData);
			break;
		}

		return in;
	}
}
