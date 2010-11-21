/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "firefoximportpage.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NewLife
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

			FirefoxImportPage::~FirefoxImportPage ()
			{

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
#ifdef Q_WS_WIN
				QString defaultFile = QDir::homePath () + "/Application Data/Mozilla/Firefox/profiles.ini";
#elif defined Q_WS_MAC
#warning Please check location of stuff on Mac OS X
				QString defaultFile = QDir::homePath () + "/.mozilla/firefox/profiles.ini";
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

			void FirefoxImportPage::handleAccepted (int index)
			{
				setField ("ProfileFile", Ui_.FileLocation_->text ());
			}
		};
	};
};
