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

			QString FirefoxImportPage::GetProfileDirectory (const QString& filename)
			{			
				if (!CheckValidity (filename))
					return QString ();
				else
				{
					QSettings settings (filename, QSettings::IniFormat);
					settings.beginGroup ("Profile0");
					QString profileDir = QDir::homePath () + "/.mozilla/firefox/" + settings.value ("Path").toString ();
					return profileDir;
				}

				return QString ();
			}


			void FirefoxImportPage::handleAccepted ()
			{				
				QString filename = Ui_.FileLocation_->text ();
				if (!CheckValidity (filename))
					return;

				QFileInfo FfStarted (GetProfileDirectory (filename) + "/lock");

				if (FfStarted.isSymLink ())
				{
					QMessageBox::critical (0,
								"LeechCraft",
								 tr ("Please close Firefox before importing."));
					return;
				}

				Entity eHistory = Util::MakeEntity (QUrl::fromLocalFile (GetProfileDirectory (filename)),
						QString (),
						FromUserInitiated,
						"x-leechcraft/browser-import-data");				
				eHistory.Additional_ ["BrowserHistory"] = GetHistory (filename);

				emit gotEntity (eHistory);
			}

			QList<QVariant> FirefoxImportPage::GetHistory (const QString& filename)
			{
				if (!CheckValidity (filename))
					return QList<QVariant> ();

				QSqlDatabase db = QSqlDatabase::addDatabase ("QSQLITE");
				QString profilePath = GetProfileDirectory (filename);

				if (!profilePath.isEmpty ())
				{
					db.setDatabaseName (profilePath + "/places.sqlite");

					if (!db.open ())
					{
						QMessageBox::critical (0,
									"LeechCraft",
									tr ("Could not open Firefox database: %1.")
									.arg (db.lastError ().text ()));
						return QList<QVariant> ();
					}
					else
					{
						QString sql ("select moz_places.url,moz_places.title, moz_historyvisits.visit_date "
								"FROM moz_historyvisits, moz_places WHERE moz_places.id = moz_historyvisits.place_id");
						QSqlQuery query (sql, db);
						QList<QVariant> history;
						while (query.next ())
						{
							QMap <QString, QVariant> record;
							record ["URL"] = query.value (0).toString ();
							record ["Title"] = query.value (1).toString ();
							record ["DateTime"] = QDateTime::fromMSecsSinceEpoch (query.value (2).toLongLong () / 1000);
							history.push_back (record);
						}
						db.close ();
						return history;
					}
				}

				return QList<QVariant> ();
			}
		};
	};
};
