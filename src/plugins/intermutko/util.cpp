/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QComboBox>
#include <QLoggingCategory>
#include <util/sll/qtutil.h>
#include <util/util.h>
#include "localeentry.h"

namespace LC::Intermutko
{
	QString GetCountryName (const QLocale& locale)
	{
		return locale.territory () == QLocale::AnyTerritory ?
			QObject::tr ("Any country") :
			locale.nativeTerritoryName ();
	}

	QString GetDisplayCode (const QLocale& locale)
	{
		const auto& name = Util::GetInternetLocaleName (locale);
		return locale.territory () != QLocale::AnyTerritory ?
				name :
				name.left (2);
	}

	void FillLanguageCombobox (QComboBox *combo)
	{
		for (int i = 0; i < QLocale::LastLanguage; ++i)
		   if (i != QLocale::C && i != QLocale::AnyLanguage)
				combo->addItem (QLocale::languageToString (static_cast<QLocale::Language> (i)), i);
	}

	void FillCountryCombobox (QComboBox *combo, QLocale::Language language)
	{
		combo->addItem (QObject::tr ("Any country"), QLocale::AnyCountry);
		QSet<QLocale::Territory> territories;
		for (const auto& locale : QLocale::matchingLocales (language, QLocale::AnyScript, QLocale::AnyTerritory))
		{
			if (territories.contains (locale.territory ()))
				continue;

			combo->addItem (GetCountryName (locale), locale.territory ());
			territories.insert (locale.territory ());
		}
	}
}
