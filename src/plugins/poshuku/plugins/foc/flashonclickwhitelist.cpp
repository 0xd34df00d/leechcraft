/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flashonclickwhitelist.h"
#include <algorithm>
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QtDebug>

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	FlashOnClickWhitelist::FlashOnClickWhitelist (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Whitelist") });

		ReadSettings ();

		Ui_.setupUi (this);
		Ui_.WhitelistTree_->setModel (Model_);
	}

	QStringList FlashOnClickWhitelist::GetWhitelist () const
	{
		QStringList result;
		for (int i = 0, rowCount = Model_->rowCount (); i < rowCount; ++i)
			result << Model_->item (i)->text ();
		return result;
	}

	bool FlashOnClickWhitelist::Matches (const QString& str) const
	{
		const auto& whitelist = GetWhitelist ();
		return std::any_of (whitelist.begin (), whitelist.end (),
				[&str] (const QString& white)
					{ return str.indexOf (white) >= 0 || str.indexOf (QRegExp { white }) >= 0; });
	}

	void FlashOnClickWhitelist::Add (const QString& str)
	{
		AddImpl (str);
	}

	void FlashOnClickWhitelist::on_Add__released ()
	{
		AddImpl ();
	}

	void FlashOnClickWhitelist::on_Modify__released ()
	{
		const auto& index = Ui_.WhitelistTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const auto& str = Model_->itemFromIndex (index)->text ();
		AddImpl (str, index);
	}

	void FlashOnClickWhitelist::on_Remove__released ()
	{
		const auto& index = Ui_.WhitelistTree_->currentIndex ();
		if (!index.isValid ())
			return;

		Model_->removeRow (index.row ());
	}

	void FlashOnClickWhitelist::accept ()
	{
		SaveSettings ();
	}

	void FlashOnClickWhitelist::reject ()
	{
		ReadSettings ();
	}

	void FlashOnClickWhitelist::AddImpl (QString str, const QModelIndex& old)
	{
		bool ok = false;
		str = QInputDialog::getText (this,
				tr ("Add URL to whitelist"),
				tr ("Please enter the URL to add to the FlashOnClick's whitelist"),
				QLineEdit::Normal,
				str,
				&ok);
		if (str.isEmpty () ||
				!ok)
			return;

		if (old.isValid ())
			Model_->removeRow (old.row ());

		if (Matches (str))
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("This URL is already matched by another whitelist entry."));
			return;
		}

		Model_->appendRow (new QStandardItem (str));
	}

	void FlashOnClickWhitelist::ReadSettings ()
	{
		if (const auto rc = Model_->rowCount ())
			Model_->removeRows (0, rc);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb");
		settings.beginGroup ("FlashOnClick");
		int size = settings.beginReadArray ("Whitelist");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			Model_->appendRow (new QStandardItem (settings.value ("Exception").toString ()));
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void FlashOnClickWhitelist::SaveSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb");
		settings.beginGroup ("FlashOnClick");
		settings.beginWriteArray ("Whitelist");
		settings.remove ("");
		for (int i = 0, rowCount = Model_->rowCount (); i < rowCount; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("Exception", Model_->item (i)->text ());
		}
		settings.endArray ();
		settings.endGroup ();
	}
}
}
}
