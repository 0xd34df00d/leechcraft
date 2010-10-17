/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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
#include <QDomDocument>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <plugininterface/util.h>
#include <QtSql>
#include <QFileInfo>
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

			int FirefoxImportPage::nextId () const
			{
				return -1;
			}

			void FirefoxImportPage::initializePage ()
			{
				connect (wizard (),
						SIGNAL (accepted ()),
						this,
						SLOT (handleAccepted ()));

				connect (this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						wizard (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				QString defaultFile = QDir::homePath () + "/.mozilla/firefox/profiles.ini";
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

			QString FirefoxImportPage::getProfileDirectory (const QString& ProfilePath)
			{
				QString filename = Ui_.FileLocation_->text ();
				if (!CheckValidity (filename))
					return "";
				else
				{
					QSettings settings (filename, QSettings::IniFormat);
					settings.beginGroup ("Profile0");
					QString profileDir = QDir::homePath () + "/.mozilla/firefox/" + settings.value ("Path").toString ();
					return profileDir;
				}

				return "";
			}


			void FirefoxImportPage::handleAccepted ()
			{				
				QString filename = Ui_.FileLocation_->text ();
				if (!CheckValidity (filename))
					return;

				QFileInfo FfStarted (getProfileDirectory (filename) + "/lock");

				if (FfStarted.isSymLink ())
				{
					QMessageBox::critical (0,
								"LeechCraft",
								 tr ("Before import close Firefox, please"));
					return;
				}

				Entity e = Util::MakeEntity (QUrl::fromLocalFile (getProfileDirectory (filename)),
						QString (),
						FromUserInitiated,
						"x-leechcraft/browser-import-data");

				setHistory ();
				if(History_.size () > 0)
					e.Additional_ ["BrowserHistory"] = History_;

//				if (Ui_.ImportSettings_->checkState () == Qt::Checked)
//				{
//					QSettings settings (QDir::homePath () + "/.kde/share/config/akregatorrc",
//							QSettings::IniFormat);
//					if (settings.status () == QSettings::NoError)
//					{
//						if (settings.contains ("Show Tray Icon"))
//							e.Additional_ ["ShowTrayIcon"] = settings.value ("Show Tray Icon");
//						if (settings.contains ("Fetch On Startup"))
//							e.Additional_ ["UpdateOnStartup"] = settings.value ("Fetch On Startup");
//						if (settings.contains ("Auto Fetch Interval"))
//							e.Additional_ ["UpdateTimeout"] = settings.value ("Auto Fetch Interval");

//						settings.beginGroup ("Archive");
//						if (settings.contains ("Max Article Number"))
//							e.Additional_ ["MaxArticles"] = settings.value ("Max Article Number");
//						if (settings.contains ("Max Article Age"))
//							e.Additional_ ["MaxAge"] = settings.value ("Max Article Age");
//						settings.endGroup ();

//						e.Additional_ ["UserVisibleName"] = tr ("Akregator settings");
//					}
//					else
//						QMessageBox::critical (0,
//								"LeechCraft",
//								tr ("Could not access or parse Akregator settings."));
//				}

//				QList <QVariant> b = e.Additional_["BrowserHistory"].toList ();
//				QMap<QString, QVariant> record;
//				for(int i = 0; i < b.size (); i++){
//					QMap<QString, QVariant> record = b[i].toMap ();
//					printf("%s:%s:%s\n", record["URL"].toString ().toStdString ().c_str (),
//							record["Title"].toString ().toStdString ().c_str (),
//							record["DateTime"].toString ().toStdString ().c_str ());
//				}				
				emit gotEntity (e);
			}

			bool FirefoxImportPage::setHistory ()
			{
				QString filename = Ui_.FileLocation_->text ();
				if (!CheckValidity (filename))
					return false;

				QSqlDatabase db = QSqlDatabase::addDatabase ("QSQLITE");
				QString profilePath = getProfileDirectory (filename);

				if (!profilePath.isEmpty ())
				{
					db.setDatabaseName (profilePath + "/places.sqlite");

					if (!db.open ())
					{
						QMessageBox::critical (0,
									"LeechCraft",
									 db.lastError ().text ());
						return false;
					}
					else
					{
						QSqlQuery query ("SELECT url, title, last_visit_date FROM moz_places;",
										QSqlDatabase::connectionNames ().value(0));

						while (query.next ())
						{
							QMap <QString, QVariant> record;
							record ["URL"] = query.value (0);
							record ["Title"] = query.value (1);
							record ["DateTime"] = query.value (2);
							History_.push_back (record);
						}
						db.close ();
					}
				}

				return true;
			}
		};
	};
};
