/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addentrydialog.h"
#include "util.h"

namespace LC::Intermutko
{
	namespace
	{
		template<typename T>
		T GetValue (const QComboBox& box)
		{
			const int idx = box.currentIndex ();
			if (idx <= 0)
				return {};

			const int val = box.itemData (idx).toInt ();
			return static_cast<T> (val);
		}
	}

	AddEntryDialog::AddEntryDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		FillLanguageCombobox (Ui_.Language_);
		Ui_.Language_->setCurrentIndex (-1);

		connect (Ui_.Language_,
				&QComboBox::currentIndexChanged,
				this,
				[this]
				{
					Ui_.Country_->clear ();
					FillCountryCombobox (Ui_.Country_, GetValue<QLocale::Language> (*Ui_.Language_));
				});
	}

	auto AddEntryDialog::GetEntries () const -> Entries
	{
		const auto country = GetValue<QLocale::Country> (*Ui_.Country_);
		const auto lang = GetValue<QLocale::Language> (*Ui_.Language_);
		const auto qval = Ui_.Q_->value ();

		Entries entries
		{
			.Specific_ = { { lang, country }, qval },
		};
		if (country != QLocale::AnyCountry)
			entries.AnyCountry_ = LocaleEntry { { lang, QLocale::AnyCountry }, qval };
		return entries;
	}
}
