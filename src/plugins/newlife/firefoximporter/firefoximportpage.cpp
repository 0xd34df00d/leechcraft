
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
				DB_.reset ( new QSqlDatabase(QSqlDatabase::addDatabase ("QSQLITE","Import connection")));
			}

			FirefoxImportPage::~FirefoxImportPage ()
			{
				QSqlDatabase::database("Import connection").close();
				QSqlDatabase::removeDatabase("Import connection");
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

				QFileInfo ffStarted (GetProfileDirectory (filename) + "/lock");

				if (ffStarted.isSymLink ())
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

				Entity eBookmarks = Util::MakeEntity (QUrl::fromLocalFile (GetProfileDirectory (filename)),
						QString (),
						FromUserInitiated,
						"x-leechcraft/browser-import-data");

				QString opmlFile = GetImportOpmlFile (filename);
				Entity eRss = Util::MakeEntity (QUrl::fromLocalFile (opmlFile),
						QString (),
						FromUserInitiated,
						"text/x-opml");

				eRss.Additional_ ["RemoveAfterHandling"] = true;
				eHistory.Additional_ ["BrowserHistory"] = GetHistory (filename);
				eBookmarks.Additional_ ["BrowserBookmarks"] = GetBookmarks (filename);

				emit gotEntity (eHistory);
				emit gotEntity (eBookmarks);
				emit gotEntity (eRss);
			}

			QList<QVariant> FirefoxImportPage::GetHistory (const QString& filename)
			{
				QString sql ("SELECT moz_places.url, moz_places.title, moz_historyvisits.visit_date "
						"FROM moz_historyvisits, moz_places WHERE moz_places.id = moz_historyvisits.place_id");
				QSqlQuery query = GetQuery (filename, sql);
				if (query.isValid ())
				{
					QList<QVariant> history;
					do
					{
						QMap<QString, QVariant> record;
						record ["URL"] = query.value (0).toString ();
						record ["Title"] = query.value (1).toString ();
						record ["DateTime"] = QDateTime::fromTime_t (query.value (2).toLongLong () / 1000000);
						history.push_back (record);
					}
					while (query.next ());
					DB_->close ();
					return history;
				}
				return QList<QVariant> ();
			}

			QList<QVariant> FirefoxImportPage::GetBookmarks (const QString& filename)
			{
				QString sql ("SELECT bm.title, pl.url FROM moz_bookmarks bm, moz_places pl "
						"WHERE bm.parent NOT IN (SELECT ann.item_id FROM moz_items_annos "
						"ann, moz_bookmarks bm WHERE ann.item_id IN (SELECT item_id FROM "
						"moz_items_annos WHERE anno_attribute_id = (SELECT id FROM "
						"moz_anno_attributes WHERE name='livemark/feedURI')) AND "
						"ann.anno_attribute_id <> 3 AND ann.anno_attribute_id <> 7 AND bm.id"
						"= ann.item_id) AND bm.fk IS NOT NULL AND bm.fk IN (SELECT id "
						"FROM moz_places WHERE url LIKE 'http%' OR url LIKE 'ftp%' OR url "
						"like 'file%') AND bm.id > 100 AND bm.fk = pl.id AND bm.title NOT NULL");
				QSqlQuery bookmarksQuery = GetQuery (filename, sql);
				if (bookmarksQuery.isValid ())
				{
					QList<QVariant> bookmarks;
					QString tagsSql_p1 ("SELECT title from moz_bookmarks WHERE id IN ("
							"SELECT bm.parent FROM moz_bookmarks bm, moz_places pl "
							" WHERE pl.url='");
					QString tagsSql_p2 ("' AND bm.title IS NULL AND bm.fk = pl.id)");
					QMap<QString, QVariant> record;
					do
					{
						QString tagsSql = tagsSql_p1 + bookmarksQuery.value (1).toString () + tagsSql_p2;
						QSqlQuery tagsQuery = GetQuery (filename, tagsSql);						

						QStringList tags;
						do
						{
							QString tag = tagsQuery.value (0).toString ();
							if (!tag.isEmpty ())
								tags << tag;
						}
						while (tagsQuery.next ());

						record ["Tags"] = tags;
						record ["Title"] = bookmarksQuery.value (0).toString ();
						record ["URL"] = bookmarksQuery.value (1).toString ();
						bookmarks.push_back (record);
					}
					while (bookmarksQuery.next ());

					DB_->close ();
					return bookmarks;
				}
				return QList<QVariant> ();
			}

			QString FirefoxImportPage::GetImportOpmlFile (const QString& filename)
			{
				QString rssSql ("SELECT ann.id, ann.item_id, ann.anno_attribute_id, ann.content,"
						"bm.title FROM moz_items_annos ann,moz_bookmarks bm WHERE ann.item_id"
						" IN (SELECT item_id FROM moz_items_annos WHERE anno_attribute_id = (SELECT"
						" id FROM moz_anno_attributes WHERE name = 'livemark/feedURI')) AND ("
						"ann.anno_attribute_id = 4 OR ann.anno_attribute_id = 5) AND "
						"bm.id = ann.item_id");
				QSqlQuery rssQuery = GetQuery (filename, rssSql);

				if (rssQuery.isValid ())
				{
					QSqlQuery query (*DB_);
					query.exec ("SELECT id FROM moz_anno_attributes WHERE name='livemark/siteURI'");
					query.next ();
					int site = query.value (0).toInt ();
					query.exec ("SELECT id FROM moz_anno_attributes WHERE name='livemark/feedURI'");
					query.next ();
					int feed = query.value (0).toInt ();

					QList<QVariant> opmlData;
					int prevItemId = -1;

					QMap<QString, QVariant> omplLine;
					do
					{
						if (rssQuery.value (2).toInt () == site)
							omplLine ["SiteUrl"] = rssQuery.value (3).toString ();
						if (rssQuery.value (2).toInt () == feed)
							omplLine ["FeedUrl"] = rssQuery.value (3).toString ();						
						if (prevItemId == rssQuery.value (1).toInt ())
							opmlData.push_back (omplLine);
						else
						{
							prevItemId = rssQuery.value (1).toInt ();
							omplLine ["Title"] = rssQuery.value (4).toString ();
						}
					}
					while (rssQuery.next ());

					QFile file ("firefox.opml");
					if (file.open (QIODevice::WriteOnly))
					{
						QXmlStreamWriter streamWriter (&file);
						streamWriter.setAutoFormatting (true);
						streamWriter.writeStartDocument ();
						streamWriter.writeStartElement ("opml");
						streamWriter.writeAttribute ("version", "1.0");
						streamWriter.writeStartElement ("head");
						streamWriter.writeStartElement ("text");
						streamWriter.writeEndElement ();
						streamWriter.writeEndElement ();
						streamWriter.writeStartElement ("body");
						streamWriter.writeStartElement ("outline");
						streamWriter.writeAttribute ("text", "Live Bookmarks");
						Q_FOREACH (const QVariant& hRowVar, opmlData)
						{
							streamWriter.writeStartElement ("outline");
							QMap<QString, QVariant> hRow = hRowVar.toMap ();
							QXmlStreamAttributes attr;
							attr.append ("title", hRow ["Title"].toString ());
							attr.append ("htmlUrl", hRow ["SiteUrl"].toString ());
							attr.append ("xmlUrl", hRow ["FeedUrl"].toString ());
							attr.append ("text", hRow ["Title"].toString ());
							streamWriter.writeAttributes (attr);
							streamWriter.writeEndElement ();
						}
						streamWriter.writeEndElement ();
						streamWriter.writeEndElement ();
						streamWriter.writeEndDocument ();

						QString filename = file.fileName ();
						file.close ();
						return filename;
					}
					else
						emit gotEntity (Util::MakeNotification ("Firefox Import",
								tr ("OPML file for importing RSS cannot be created: %1")
									.arg (file.errorString ()),
								PCritical_));
				}
				return QString ();
			}

			QSqlQuery FirefoxImportPage::GetQuery (const QString& filename, const QString& sql)
			{
				if (!CheckValidity (filename))
					return QSqlQuery ();

				QString profilePath = GetProfileDirectory (filename);

				if (profilePath.isEmpty ())
					return QSqlQuery ();

				DB_->setDatabaseName (profilePath + "/places.sqlite");

				if (!DB_->open ())
				{
					qWarning () << Q_FUNC_INFO
							<< "could not open database"
							<< DB_->lastError ().text ();
					emit gotEntity (Util::MakeNotification (tr ("Firefox Import"),
								tr ("Could not open Firefox database: %1.")
									.arg (DB_->lastError ().text ()),
								PCritical_));
				}
				else
				{
					QSqlQuery query (*DB_);
					query.exec (sql);
					if (query.isActive ())
					{
						query.next ();
						return query;
					}
				}
				return QSqlQuery ();
			}
		};
	};
};
