/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "acceptlangwidget.h"
#include <QMessageBox>
#include <util/sll/prelude.h>
#include "xmlsettingsmanager.h"
#include "localesmodel.h"
#include "entriesdelegate.h"
#include "util.h"

Q_DECLARE_METATYPE (QList<QLocale>)

namespace LC
{
namespace Intermutko
{
	AcceptLangWidget::AcceptLangWidget (QWidget *parent)
	: QWidget { parent }
	, Model_ { new LocalesModel { this } }
	{
		Ui_.setupUi (this);
		Ui_.LangsTree_->setItemDelegate (new EntriesDelegate);
		Ui_.LangsTree_->setModel (Model_);

		LoadSettings ();

		reject ();

		for (int i = 0; i < QLocale::LastTerritory; ++i)
			Ui_.Country_->addItem (QLocale::territoryToString (static_cast<QLocale::Country> (i)), i);

		FillLanguageCombobox (Ui_.Language_);
		on_Language__currentIndexChanged (Ui_.Language_->currentIndex ());
	}

	const QString& AcceptLangWidget::GetLocaleString () const
	{
		return LocaleStr_;
	}

	namespace
	{
		QString GetFullCode (const LocaleEntry& entry)
		{
			return GetDisplayCode (entry.Locale_) + ";q=" + QString::number (entry.Q_);
		}
	}

	void AcceptLangWidget::AddLocale (const LocaleEntry& entry)
	{
		if (Model_->GetEntries ().contains (entry))
			return;

		Model_->AddLocaleEntry (entry);
	}

	void AcceptLangWidget::WriteSettings ()
	{
		XmlSettingsManager::Instance ().setProperty ("LocaleEntries", QVariant::fromValue (Locales_));
	}

	namespace
	{
		QList<LocaleEntry> BuildDefaultLocaleList ()
		{
			QList<LocaleEntry> result;

			const QLocale defLocale;
			result.append ({ defLocale, 1 });
			if (defLocale.territory () != QLocale::AnyTerritory)
				result.append ({ QLocale { defLocale.language (), QLocale::AnyCountry }, 0.9 });

			return result;
		}
	}

	void AcceptLangWidget::LoadSettings ()
	{
		const auto& localesVar = XmlSettingsManager::Instance ().property ("LocaleEntries");
		if (!localesVar.isNull ())
			Locales_ = localesVar.value<QList<LocaleEntry>> ();
		else
		{
			Locales_ = BuildDefaultLocaleList ();
			WriteSettings ();
		}

		RebuildLocaleStr ();
	}

	void AcceptLangWidget::RebuildLocaleStr ()
	{
		LocaleStr_ = QStringList { Util::Map (Locales_, &GetFullCode) }.join (", ");
	}

	void AcceptLangWidget::accept ()
	{
		Locales_ = Model_->GetEntries ();
		WriteSettings ();
		RebuildLocaleStr ();
	}

	void AcceptLangWidget::reject ()
	{
		Model_->SetLocales (Locales_);
	}

	namespace
	{
		template<typename T>
		T GetValue (const QComboBox *box)
		{
			const int idx = box->currentIndex ();
			if (idx <= 0)
				return {};

			const int val = box->itemData (idx).toInt ();
			return static_cast<T> (val);
		}
	}

	void AcceptLangWidget::on_Add__released ()
	{
		const auto country = GetValue<QLocale::Country> (Ui_.Country_);
		const auto lang = GetValue<QLocale::Language> (Ui_.Language_);
		const auto qval = Ui_.Q_->value ();
		AddLocale ({ { lang, country }, qval });

		if (!Model_->GetEntries ().contains ({ { lang, QLocale::AnyCountry }, qval }) &&
				QMessageBox::question (this,
						"LeechCraft",
						tr ("Do you want to add an accepted language without "
							"any country specified as a fallback?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			AddLocale ({ { lang, QLocale::AnyCountry }, qval });
	}

	void AcceptLangWidget::on_Remove__released ()
	{
		Model_->Remove (Ui_.LangsTree_->currentIndex ());
	}

	void AcceptLangWidget::on_MoveUp__released ()
	{
		Model_->MoveUp (Ui_.LangsTree_->currentIndex ());
	}

	void AcceptLangWidget::on_MoveDown__released ()
	{
		Model_->MoveDown (Ui_.LangsTree_->currentIndex ());
	}

	void AcceptLangWidget::on_Language__currentIndexChanged (int)
	{
		Ui_.Country_->clear ();

		const auto lang = GetValue<QLocale::Language> (Ui_.Language_);
		FillCountryCombobox (Ui_.Country_, lang);
	}
}
}
