/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLocale>

namespace LC::Intermutko
{
	struct LocaleEntry
	{
		QLocale Locale_;
		double Q_;
	};

	bool operator== (const LocaleEntry&, const LocaleEntry&);

	QDataStream& operator<< (QDataStream&, const LocaleEntry&);
	QDataStream& operator>> (QDataStream&, LocaleEntry&);
}

Q_DECLARE_METATYPE (LC::Intermutko::LocaleEntry)

Q_DECLARE_METATYPE (QList<LC::Intermutko::LocaleEntry>)
