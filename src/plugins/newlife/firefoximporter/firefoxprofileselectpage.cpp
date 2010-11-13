/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "firefoxprofileselectpage.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>
#include <QUrl>
#include <QDateTime>
#include <QXmlStreamWriter>

#include <plugininterface/util.h>
#include "boost/bind.hpp"


namespace LeechCraft
{
	namespace Plugins
	{
		namespace NewLife
		{
			FirefoxProfileSelectPage::FirefoxProfileSelectPage (QWidget *parent)
			: QWizardPage (parent)
			{
				Ui_.setupUi (this);
 				DB_.reset (new QSqlDatabase (QSqlDatabase::addDatabase ("QSQLITE","Import connection")));
			}

			FirefoxProfileSelectPage::~FirefoxProfileSelectPage ()
			{
				QSqlDatabase::database ("Import connection").close ();			
				DB_.reset ();
				QSqlDatabase::removeDatabase ("Import connection");
			}
			
			int FirefoxProfileSelectPage::nextId () const
			{
				return -1;
			}
			
			void FirefoxProfileSelectPage::initializePage ()
			{
				connect (wizard (),
						SIGNAL (accepted ()),
						this,
						SLOT (handleAccepted ()));

				connect (this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						wizard (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				
				connect (Ui_.ProfileList_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (checkImportDataAvailable (int)));
				
 				GetProfileList (field ("ProfileFile").toString ());
			}
			
			void FirefoxProfileSelectPage::GetProfileList (const QString& filename)
			{
				QSettings settings (filename, QSettings::IniFormat);
				Ui_.ProfileList_->clear ();
				Ui_.ProfileList_->addItem ("");
				Q_FOREACH (const QString& name, settings.childGroups ())
				{
					settings.beginGroup (name);
					Ui_.ProfileList_->addItem (settings.value ("Name").toString ());
					settings.endGroup ();
				}
			}

			void FirefoxProfileSelectPage::checkImportDataAvailable (int index)
			{
				Ui_.HistoryImport_->setChecked (false);
				Ui_.BookmarksImport_->setChecked (false);
				Ui_.RssImport_->setChecked (false);

				if (!index){
					Ui_.HistoryImport_->setEnabled (false);
					Ui_.BookmarksImport_->setEnabled (false);
					Ui_.RssImport_->setEnabled (false);
					return;
				}

				if (IsFirefoxRunning ())
				{
					QMessageBox::critical (0,
							"LeechCraft",
							 tr ("Please close Firefox before importing."));
					Ui_.ProfileList_->setCurrentIndex (0);
					return;
				}
				
				QString profilePath = GetProfileDirectory (Ui_.ProfileList_->currentText ());
				
				QString rssSql ("SELECT COUNT(ann.id) FROM moz_items_annos ann,moz_bookmarks bm " 
						"WHERE ann.item_id IN (SELECT item_id FROM moz_items_annos WHERE "
						" anno_attribute_id = (SELECT id FROM moz_anno_attributes WHERE name "
						"= 'livemark/feedURI')) AND (ann.anno_attribute_id = 4 OR "
						"ann.anno_attribute_id = 5) AND bm.id = ann.item_id");
				QString bookmarksSql ("SELECT COUNT(pl.url) FROM moz_bookmarks bm, moz_places pl "
						"WHERE bm.parent NOT IN (SELECT ann.item_id FROM moz_items_annos "
						"ann, moz_bookmarks bm WHERE ann.item_id IN (SELECT item_id FROM "
						"moz_items_annos WHERE anno_attribute_id = (SELECT id FROM "
						"moz_anno_attributes WHERE name='livemark/feedURI')) AND "
						"ann.anno_attribute_id <> 3 AND ann.anno_attribute_id <> 7 AND bm.id"
						"= ann.item_id) AND bm.fk IS NOT NULL AND bm.fk IN (SELECT id "
						"FROM moz_places WHERE url LIKE 'http%' OR url LIKE 'ftp%' OR url "
						"like 'file%') AND bm.id > 100 AND bm.fk = pl.id AND bm.title NOT NULL");
				QString historySql ("SELECT COUNT(moz_places.url) FROM moz_historyvisits," 
						"moz_places WHERE moz_places.id = moz_historyvisits.place_id");
				
				if (profilePath.isEmpty ())
					return;
				
				QSqlQuery query = GetQuery (bookmarksSql);
				QSqlRecord record = query.record();
				int count = record.value (0).toInt ();
				
				if (!count)
					Ui_.BookmarksImport_->setEnabled (false);
				else
					Ui_.BookmarksImport_->setEnabled (true);
				
				query = GetQuery (historySql);
				record = query.record();
				count = record.value (0).toInt ();
				
				if (!count)
					Ui_.HistoryImport_->setEnabled (false);
				else
					Ui_.HistoryImport_->setEnabled (true);
				
				query = GetQuery (bookmarksSql);
				record = query.record();
				count = record.value (0).toInt ();
				
				if (!count)
					Ui_.RssImport_->setEnabled (false);
				else
					Ui_.RssImport_->setEnabled (true);
			}

			
			QString FirefoxProfileSelectPage::GetProfileDirectory (const QString& profileName) const
			{
				QString profilesFile = field ("ProfileFile").toString ();
				QSettings settings (profilesFile, QSettings::IniFormat);
				QString profilePath;
				Q_FOREACH (const QString& groupName, settings.childGroups ())
				{
					// Call settings.endGroup() on scope exit no matter what.
					boost::shared_ptr<void> guard (static_cast<void*> (0), 
							boost::bind (&QSettings::endGroup, &settings));
					settings.beginGroup (groupName);
					if (settings.value ("Name").toString () == profileName)
					{	
						profilePath = settings.value ("Path").toString ();
						break;
					}		
				}
				if (profilePath.isEmpty ())
					return QString ();
				
				QFileInfo file (profilesFile);
				profilePath = file.absolutePath  ().append ("/").append (profilePath);
				
				return profilePath; 
			}
			
			QList<QVariant> FirefoxProfileSelectPage::GetHistory ()
			{
				QString sql ("SELECT moz_places.url, moz_places.title, moz_historyvisits.visit_date "
						"FROM moz_historyvisits, moz_places WHERE moz_places.id = moz_historyvisits.place_id");
				QSqlQuery query = GetQuery (sql);
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
					return history;
				}
				return QList<QVariant> ();
			}

			QList<QVariant> FirefoxProfileSelectPage::GetBookmarks ()
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
				QSqlQuery bookmarksQuery = GetQuery (sql);
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
						QSqlQuery tagsQuery = GetQuery (tagsSql);

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

					return bookmarks;
				}
				return QList<QVariant> ();
			}

			QString FirefoxProfileSelectPage::GetImportOpmlFile ()
			{
				QString rssSql ("SELECT ann.id, ann.item_id, ann.anno_attribute_id, ann.content,"
						"bm.title FROM moz_items_annos ann,moz_bookmarks bm WHERE ann.item_id"
						" IN (SELECT item_id FROM moz_items_annos WHERE anno_attribute_id = (SELECT"
						" id FROM moz_anno_attributes WHERE name = 'livemark/feedURI')) AND ("
						"ann.anno_attribute_id = 4 OR ann.anno_attribute_id = 5) AND "
						"bm.id = ann.item_id");
				QSqlQuery rssQuery = GetQuery (rssSql);

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

			QSqlQuery FirefoxProfileSelectPage::GetQuery (const QString& sql)
			{
				QString profilePath = GetProfileDirectory (Ui_.ProfileList_->currentText ());

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
			
			void FirefoxProfileSelectPage::handleAccepted ()
			{	
				if (IsFirefoxRunning ())
					return;
					
				if (Ui_.HistoryImport_->isEnabled () && Ui_.HistoryImport_->isChecked ())
				{
					Entity eHistory = Util::MakeEntity (QUrl::fromLocalFile (GetProfileDirectory (Ui_.ProfileList_->currentText ())),
						QString (),
						FromUserInitiated,
						"x-leechcraft/browser-import-data");
				
					eHistory.Additional_ ["BrowserHistory"] = GetHistory ();
					emit gotEntity (eHistory);
				}
				
				if (Ui_.BookmarksImport_->isEnabled () && Ui_.BookmarksImport_->isChecked ())
				{
					Entity eBookmarks = Util::MakeEntity (QUrl::fromLocalFile (GetProfileDirectory (Ui_.ProfileList_->currentText ())),
							QString (),
							FromUserInitiated,
							"x-leechcraft/browser-import-data");
				
					eBookmarks.Additional_ ["BrowserBookmarks"] = GetBookmarks ();
					emit gotEntity (eBookmarks);
				}
				
				if (Ui_.RssImport_->isEnabled () && Ui_.RssImport_->isChecked ())
				{
					QString opmlFile = GetImportOpmlFile ();
					Entity eRss = Util::MakeEntity (QUrl::fromLocalFile (opmlFile),
							QString (),
							FromUserInitiated,
							"text/x-opml");

					eRss.Additional_ ["RemoveAfterHandling"] = true;
					emit gotEntity (eRss);
				}
				DB_->close ();
			}
			
			bool FirefoxProfileSelectPage::IsFirefoxRunning()
			{
				
				QFileInfo ffStarted (GetProfileDirectory (Ui_.ProfileList_->currentText ()) + "/lock");

				if (ffStarted.isSymLink ())
					return true;
				
				return false;
			}
		};
	};
};