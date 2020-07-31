/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "firefoximportpage.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>

namespace LC
{
namespace NewLife
{
namespace Importers
{
	FirefoxImportPage::FirefoxImportPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		Ui_.ImportSettings_->setText (Ui_.ImportSettings_->text ().arg ("Firefox"));

		setTitle (tr ("Firefox's data import"));
		setSubTitle (tr ("Select Firefox's INI file"));
		registerField ("ProfileFile", Ui_.FileLocation_);
	}

	bool FirefoxImportPage::CheckValidity (const QString& filename) const
	{
		QFile file (filename);
		if (!file.exists () ||
				!file.open (QIODevice::ReadOnly))
			return false;
		return true;
	}

	bool FirefoxImportPage::isComplete () const
	{
		return CheckValidity (Ui_.FileLocation_->text ());
	}

	void FirefoxImportPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (currentIdChanged (int)),
				this,
				SLOT (handleAccepted (int)));
#ifdef Q_OS_WIN32
		QString defaultFile = QDir::homePath () + "/Application Data/Mozilla/Firefox/profiles.ini";
#elif defined Q_OS_MAC
#warning Please check location of stuff on Mac OS X
		QString defaultFile = QDir::homePath () + "/Library/Application Support/Firefox/profiles.ini";
#else
		QString defaultFile = QDir::homePath () + "/.mozilla/firefox/profiles.ini";
#endif
		if (CheckValidity (defaultFile))
			Ui_.FileLocation_->setText (defaultFile);
	}

	void FirefoxImportPage::on_Browse__released ()
	{
		QString filename = QFileDialog::getOpenFileName (this,
				tr ("Select Firefox's INI file"),
				QDir::homePath () + "/.mozilla/",
				tr ("INI files (*.ini);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		if (!CheckValidity (filename))
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("The file you've selected is not a valid INI file."));
		else
			Ui_.FileLocation_->setText (filename);

		emit completeChanged ();
	}

	void FirefoxImportPage::on_FileLocation__textEdited (const QString&)
	{
		emit completeChanged ();
	}

	void FirefoxImportPage::handleAccepted (int)
	{
		setField ("ProfileFile", Ui_.FileLocation_->text ());
	}
}
}
}
