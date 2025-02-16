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
#include "addentrydialog.h"
#include "xmlsettingsmanager.h"
#include "localesmodel.h"
#include "entriesdelegate.h"
#include "util.h"

namespace LC::Intermutko
{
	AcceptLangWidget::AcceptLangWidget (QWidget *parent)
	: QWidget { parent }
	, Model_ { new LocalesModel { this } }
	{
		Ui_.setupUi (this);
		Ui_.LangsTree_->setItemDelegate (new EntriesDelegate);
		Ui_.LangsTree_->setModel (Model_);

		LoadSettings ();

		Model_->SetLocales (Locales_);

		connect (Ui_.Add_,
				&QPushButton::released,
				this,
				&AcceptLangWidget::RunAddLocaleDialog);
		connect (Ui_.Remove_,
				&QPushButton::released,
				this,
				[this] { Model_->Remove (Ui_.LangsTree_->currentIndex ()); });
		connect (Ui_.MoveUp_,
				&QPushButton::released,
				this,
				[this] { Model_->MoveUp (Ui_.LangsTree_->currentIndex ()); });
		connect (Ui_.MoveDown_,
				&QPushButton::released,
				this,
				[this] { Model_->MoveDown (Ui_.LangsTree_->currentIndex ()); });
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

	void AcceptLangWidget::RunAddLocaleDialog ()
	{
		AddEntryDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& entries = dia.GetEntries ();
		Model_->AddLocaleEntry (entries.Specific_);

		if (entries.AnyCountry_ &&
			!Model_->GetEntries ().contains (*entries.AnyCountry_) &&
			QMessageBox::question (this,
					"LeechCraft",
					tr ("Do you want to add this language without any country specified as a fallback?"),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			Model_->AddLocaleEntry (*entries.AnyCountry_);
	}
}
