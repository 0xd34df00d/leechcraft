/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLocale>

class QComboBox;

namespace LC::Intermutko
{
	struct LocaleEntry;

	QString GetCountryName (const QLocale&);
	QString GetDisplayCode (const QLocale&);

	void FillLanguageCombobox (QComboBox *combo);
	void FillCountryCombobox (QComboBox *combo, QLocale::Language language);
}
