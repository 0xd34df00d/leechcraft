/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QComboBox>
#include <util/sll/qtutil.h>

namespace LC::Intermutko
{
	QString GetCountryName (QLocale::Language language, QLocale::Territory territory)
	{
		return territory == QLocale::AnyTerritory ?
				QObject::tr ("Any country") :
				"%1 (%2)"_qs.arg (QLocale { language, territory }.nativeTerritoryName (), QLocale::territoryToString (territory));
	}

	QString GetDisplayCode (QLocale::Language language, QLocale::Territory territory)
	{
		const auto& code = QLocale::languageToCode (language);
		return territory == QLocale::AnyTerritory ?
				code :
				code + u'-' + QLocale::territoryToCode (territory);
	}

	void FillLanguageCombobox (QComboBox *combo)
	{
		QSet<QLocale::Language> languages { QLocale::AnyLanguage, QLocale::C };
		for (const auto& locale : QLocale::matchingLocales (QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyTerritory))
		{
			const auto lang = locale.language ();
			if (languages.contains (lang))
				continue;

			languages.insert (lang);

			const auto& nativeName = locale.nativeLanguageName ();
			const auto& engName = QLocale::languageToString (lang);
			combo->addItem ("%1 (%2)"_qs.arg (nativeName, engName), lang);
		}
	}

	void FillCountryCombobox (QComboBox *combo, QLocale::Language language)
	{
		combo->addItem (QObject::tr ("Any country"), QLocale::AnyTerritory);
		QSet<QLocale::Territory> territories;
		for (const auto& locale : QLocale::matchingLocales (language, QLocale::AnyScript, QLocale::AnyTerritory))
		{
			if (territories.contains (locale.territory ()))
				continue;

			combo->addItem (GetCountryName (language, locale.territory ()), locale.territory ());
			territories.insert (locale.territory ());
		}
	}
}
