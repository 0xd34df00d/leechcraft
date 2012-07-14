/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "acceptlangwidget.h"
#include <QStandardItemModel>
#include "util/util.h"
#include "networkaccessmanager.h"
#include "core.h"

namespace LeechCraft
{
	AcceptLangWidget::AcceptLangWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.LangsTree_->setModel (Model_);

		connect (Core::Instance ().GetNetworkAccessManager (),
				SIGNAL (acceptableLanguagesChanged ()),
				this,
				SLOT (reject ()));
		reject ();

		for (int i = 0; i < QLocale::LastCountry; ++i)
			Ui_.Country_->addItem (QLocale::countryToString (static_cast<QLocale::Country> (i)), i);
		Ui_.Country_->model ()->sort (0);

		for (int i = 0; i < QLocale::LastLanguage; ++i)
		{
			if (i == QLocale::C)
				continue;

			Ui_.Language_->addItem (QLocale::languageToString (static_cast<QLocale::Language> (i)), i);
		}
		Ui_.Language_->model ()->sort (0);
		on_Language__currentIndexChanged (Ui_.Language_->currentIndex ());
	}

	void AcceptLangWidget::AddLocale (const QLocale& locale)
	{
		QList<QStandardItem*> items;
		items << new QStandardItem (QLocale::languageToString (locale.language ()));
		items << new QStandardItem (QLocale::countryToString (locale.country ()));
		items << new QStandardItem (Util::GetInternetLocaleName (locale));
		Model_->appendRow (items);
		items.first ()->setData (locale, Roles::LocaleObj);
	}
	
	void AcceptLangWidget::accept ()
	{
		QList<QLocale> locales;
		for (int i = 0; i < Model_->rowCount (); ++i)
			locales << Model_->item (i)->data (Roles::LocaleObj).toLocale ();

		auto qnam = Core::Instance ().GetNetworkAccessManager ();
		auto nam = qobject_cast<NetworkAccessManager*> (qnam);
		nam->SetAcceptLangs (locales);
	}

	void AcceptLangWidget::reject ()
	{
		Model_->clear ();
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Language")) << tr ("Country") << tr ("Code"));

		auto qnam = Core::Instance ().GetNetworkAccessManager ();
		auto nam = qobject_cast<NetworkAccessManager*> (qnam);
		Q_FOREACH (const QLocale& locale, nam->GetAcceptLangs ())
			AddLocale (locale);
	}

	namespace
	{
		template<typename T>
		T GetValue (const QComboBox *box)
		{
			const int idx = box->currentIndex ();
			if (idx <= 0)
				return T ();

			const int val = box->itemData (idx).toInt ();
			return static_cast<T> (val);
		}
	}

	void AcceptLangWidget::on_Add__released ()
	{
		const auto country = GetValue<QLocale::Country> (Ui_.Country_);
		const auto lang = GetValue<QLocale::Language> (Ui_.Language_);
		AddLocale (QLocale (lang, country));
	}

	void AcceptLangWidget::on_Remove__released ()
	{
		const auto& idx = Ui_.LangsTree_->currentIndex ();
		if (!idx.isValid ())
			return;

		Model_->removeRow (idx.row ());
	}

	void AcceptLangWidget::on_MoveUp__released ()
	{
		QStandardItem *item = Model_->itemFromIndex (Ui_.LangsTree_->currentIndex ());
		if (!item || !item->row ())
			return;

		const int row = item->row ();
		Model_->insertRow (row - 1, Model_->takeRow (row));
	}

	void AcceptLangWidget::on_MoveDown__released ()
	{
		QStandardItem *item = Model_->itemFromIndex (Ui_.LangsTree_->currentIndex ());
		if (!item || item->row () == Model_->rowCount () - 1)
			return;

		const int row = item->row ();
		auto items = Model_->takeRow (row);
		Model_->insertRow (row + 1, items);
	}

	void AcceptLangWidget::on_Language__currentIndexChanged (int)
	{
		Ui_.Country_->clear ();

		const auto lang = GetValue<QLocale::Language> (Ui_.Language_);
		auto countries = QLocale::countriesForLanguage (lang);
		if (!countries.contains (QLocale::AnyCountry))
			countries << QLocale::AnyCountry;
		Q_FOREACH (QLocale::Country c, countries)
			Ui_.Country_->addItem (QLocale::countryToString (c), c);
		Ui_.Country_->model ()->sort (0);
	}
}
