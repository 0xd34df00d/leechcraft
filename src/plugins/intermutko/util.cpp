/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QComboBox>
#include <util/util.h>
#include "localeentry.h"

namespace LC
{
namespace Intermutko
{
	QString GetCountryName (QLocale::Language lang, QLocale::Country country)
	{
		return country == QLocale::AnyCountry ?
				QObject::tr ("Any country") :
				QLocale {lang, country }.nativeCountryName ();
	}

	QString GetDisplayCode (const LocaleEntry& entry)
	{
		const auto& name = Util::GetInternetLocaleName ({ entry.Language_, entry.Country_ });
		return entry.Country_ != QLocale::AnyCountry ?
				name :
				name.left (2);
	}

	void FillLanguageCombobox (QComboBox *combo)
	{
		for (int i = 0; i < QLocale::LastLanguage; ++i)
		{
			if (i == QLocale::C)
				continue;

			combo->addItem (QLocale::languageToString (static_cast<QLocale::Language> (i)), i);
		}
		combo->model ()->sort (0);
	}

	void FillCountryCombobox (QComboBox *combo, QLocale::Language language)
	{
		auto countries = QLocale::countriesForLanguage (language);
		if (!countries.contains (QLocale::AnyCountry))
			countries << QLocale::AnyCountry;

		for (auto c : countries)
			combo->addItem (GetCountryName (language, c), c);

		combo->model ()->sort (0);
	}
}
}
