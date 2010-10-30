/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "sqlstoragebackend_mysql.h"
#include <stdexcept>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>
#include <plugininterface/dblock.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			SQLStorageBackendMysql::SQLStorageBackendMysql (StorageBackend::Type type)
			: Type_ (type)
			{
				DB_ = QSqlDatabase::addDatabase ("QMYSQL", "PoshukuConnection");
				DB_.setDatabaseName (XmlSettingsManager::Instance ()->
						property ("MySQLDBName").toString ());
				DB_.setHostName (XmlSettingsManager::Instance ()->
						property ("MySQLHostname").toString ());
				DB_.setPort (XmlSettingsManager::Instance ()->
						property ("MySQLPort").toInt ());
				DB_.setUserName (XmlSettingsManager::Instance ()->
						property ("MySQLUsername").toString ());
				DB_.setPassword (XmlSettingsManager::Instance ()->
						property ("MySQLPassword").toString ());

				if (!DB_.open ())
				{
					LeechCraft::Util::DBLock::DumpError (DB_.lastError ());
					throw std::runtime_error (QString ("Could not initialize database: %1")
							.arg (DB_.lastError ().text ()).toUtf8 ().constData ());
				}

				InitializeTables ();
			}

			SQLStorageBackendMysql::~SQLStorageBackendMysql ()
			{

			}

			void SQLStorageBackendMysql::Prepare ()
			{

				HistoryLoader_ = QSqlQuery (DB_);
				HistoryLoader_.prepare ("SELECT "
						"title, "
						"date, "
						"url "
						"FROM history "
						"ORDER BY date DESC");

				HistoryRatedLoader_ = QSqlQuery (DB_);
				HistoryRatedLoader_.prepare ("SELECT "
						"SUM (AGE (date)) - AGE (MIN (date)) * COUNT (date) AS rating, "
						"MAX (title) AS title, "
						"url "
						"FROM history "
						"WHERE ( title LIKE ? ) "
						"OR ( url LIKE ? ) "
						"GROUP BY url "
						"ORDER BY rating ASC "
						"LIMIT 100");

				HistoryAdder_ = QSqlQuery (DB_);
				HistoryAdder_.prepare ("INSERT INTO history ("
						"date, "
						"title, "
						"url"
						") VALUES ("
						"? , "
						"? , "
						"? "
						")");

				HistoryEraser_ = QSqlQuery (DB_);
				HistoryEraser_.prepare ("DELETE FROM history "
						"WHERE "
						" DATE_ADD(date, INTERVAL ? DAY) < now () )");

				HistoryTruncater_ = QSqlQuery (DB_);
				HistoryTruncater_.prepare ("DELETE FROM history "
						"WHERE date IN "
						"(SELECT date FROM history ORDER BY date DESC "
						"LIMIT 10000 OFFSET ?)");

				FavoritesLoader_ = QSqlQuery (DB_);
				FavoritesLoader_.prepare ("SELECT "
						"title, "
						"url, "
						"tags "
						"FROM favorites "
						"ORDER BY ROWID DESC");

				FavoritesAdder_ = QSqlQuery (DB_);
				FavoritesAdder_.prepare ("INSERT INTO favorites ("
						"title, "
						"url, "
						"tags"
						") VALUES ("
						"?, "
						"?, "
						"?"
						")");

				FavoritesUpdater_ = QSqlQuery (DB_);
				FavoritesUpdater_.prepare ("UPDATE favorites SET "
						"title = ?, "
						"tags = ? "
						"WHERE url = ?");

				FavoritesRemover_ = QSqlQuery (DB_);
				FavoritesRemover_.prepare ("DELETE FROM favorites "
						"WHERE url = ?");

				FormsIgnoreSetter_ = QSqlQuery (DB_);
				FormsIgnoreSetter_.prepare ("INSERT INTO forms_never ("
						"url"
						") VALUES ("
						" ? "
						")");

				FormsIgnoreGetter_ = QSqlQuery (DB_);
				FormsIgnoreGetter_.prepare ("SELECT COUNT (url) AS num "
						"FROM forms_never "
						"WHERE url = ? ");

				FormsIgnoreClearer_ = QSqlQuery (DB_);
				FormsIgnoreClearer_.prepare ("DELETE FROM forms_never ("
						"WHERE url = ? ");
			}

			void SQLStorageBackendMysql::LoadHistory (history_items_t& items) const
			{
				if (!HistoryLoader_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (HistoryLoader_);
					return;
				}

				while (HistoryLoader_.next ())
				{
					HistoryItem item =
					{
						HistoryLoader_.value (0).toString (),
						HistoryLoader_.value (1).toDateTime (),
						HistoryLoader_.value (2).toString ()
					};
					items.push_back (item);
				}

				HistoryLoader_.finish ();
			}

			void SQLStorageBackendMysql::LoadResemblingHistory (const QString& base,
					history_items_t& items) const
			{
				QString bound = "%";
				bound += base;
				bound += "%";
				HistoryRatedLoader_.bindValue (0, bound);
				HistoryRatedLoader_.bindValue (1, bound);
				if (!HistoryRatedLoader_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (HistoryRatedLoader_);
					throw std::runtime_error ("failed to load ratedly");
				}

				while (HistoryRatedLoader_.next ())
				{
					HistoryItem item =
					{
						HistoryRatedLoader_.value (1).toString (),
						QDateTime (),
						HistoryRatedLoader_.value (2).toString ()
					};
					items.push_back (item);
				}
				HistoryRatedLoader_.finish ();
			}

			void SQLStorageBackendMysql::AddToHistory (const HistoryItem& item)
			{
				HistoryAdder_.bindValue (0, item.Title_);
				HistoryAdder_.bindValue (1, item.DateTime_);
				HistoryAdder_.bindValue (2, item.URL_);

				if (!HistoryAdder_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (HistoryAdder_);
					return;
				}

				emit added (item);
			}

			void SQLStorageBackendMysql::ClearOldHistory (int age, int items)
			{
				LeechCraft::Util::DBLock lock (DB_);
				lock.Init ();
				HistoryEraser_.bindValue (0, age);
				HistoryTruncater_.bindValue (1, items);

				if (!HistoryEraser_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (HistoryEraser_);
					return;
				}
				if (!HistoryTruncater_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (HistoryTruncater_);
					return;
				}

				lock.Good ();
			}

			void SQLStorageBackendMysql::LoadFavorites (
					FavoritesModel::items_t& items
					) const
			{
				if (!FavoritesLoader_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FavoritesLoader_);
					return;
				}

				while (FavoritesLoader_.next ())
				{
					FavoritesModel::FavoritesItem item =
					{
						FavoritesLoader_.value (0).toString (),
						FavoritesLoader_.value (1).toString (),
						FavoritesLoader_.value (2).toString ().split (" ",
								QString::SkipEmptyParts)
					};
					items.push_back (item);
				}

				FavoritesLoader_.finish ();
			}

			void SQLStorageBackendMysql::AddToFavorites (const FavoritesModel::FavoritesItem& item)
			{
				FavoritesAdder_.bindValue (0, item.Title_);
				FavoritesAdder_.bindValue (1, item.URL_);
				FavoritesAdder_.bindValue (2, item.Tags_.join (" "));

				if (!FavoritesAdder_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FavoritesAdder_);
					throw std::runtime_error ("Failed to execute FavoritesAdder query.");
				}

				emit added (item);
			}

			void SQLStorageBackendMysql::RemoveFromFavorites (const FavoritesModel::FavoritesItem& item)
			{
				FavoritesRemover_.bindValue (0, item.URL_);
				if (!FavoritesRemover_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FavoritesRemover_);
					return;
				}

				emit removed (item);
			}

			void SQLStorageBackendMysql::UpdateFavorites (const FavoritesModel::FavoritesItem& item)
			{
				FavoritesUpdater_.bindValue (0, item.Title_);
				FavoritesUpdater_.bindValue (1, item.URL_);
				FavoritesUpdater_.bindValue (2, item.Tags_.join (" "));

				if (!FavoritesUpdater_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FavoritesUpdater_);
					return;
				}

				emit updated (item);
			}

			void SQLStorageBackendMysql::SetFormsIgnored (const QString& url, bool ignore)
			{
				if (ignore)
				{
					FormsIgnoreSetter_.bindValue (0, url);
					if (!FormsIgnoreSetter_.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (FormsIgnoreSetter_);
						return;
					}
				}
				else
				{
					FormsIgnoreClearer_.bindValue (0, url);
					if (!FormsIgnoreClearer_.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (FormsIgnoreClearer_);
						return;
					}
				}
			}

			bool SQLStorageBackendMysql::GetFormsIgnored (const QString& url) const
			{
				FormsIgnoreGetter_.bindValue (0, url);
				if (!FormsIgnoreGetter_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FormsIgnoreGetter_);
					return false;
				}

				FormsIgnoreGetter_.next ();

				bool ignored = FormsIgnoreGetter_.value (0).toInt ();
				FormsIgnoreGetter_.finish ();
				return ignored;
			}

			void SQLStorageBackendMysql::InitializeTables ()
			{
				QSqlQuery query (DB_);

				if (!DB_.tables ().contains ("history"))
				{
					if (!query.exec ("CREATE TABLE history ("
								"date TIMESTAMP PRIMARY KEY, "
								"title TEXT, "
								"url TEXT"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return;
					}

					if (!query.exec ("CREATE INDEX idx_history_title_url "
								"ON history (title, url)"))
						LeechCraft::Util::DBLock::DumpError (query);
				}

				if (!DB_.tables ().contains ("favorites"))
				{
					if (!query.exec ("CREATE TABLE favorites ("
								"title TEXT PRIMARY KEY, "
								"url TEXT, "
								"tags TEXT"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return;
					}
				}

				if (!DB_.tables ().contains ("storage_settings"))
				{
					if (!query.exec ("CREATE TABLE storage_settings ("
								"key TEXT PRIMARY KEY, "
								"value TEXT"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return;
					}

					SetSetting ("historyversion", "1");
					SetSetting ("favoritesversion", "1");
					SetSetting ("storagesettingsversion", "1");
				}

				if (!DB_.tables ().contains ("forms"))
				{
					QString binary = "BLOB";

					if (!query.exec (QString ("CREATE TABLE forms ("
									"url TEXT, "
									"form_index INTEGER, "
									"name TEXT, "
									"type TEXT, "
									"value %1"
									");").arg (binary)))
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return;
					}
				}

				if (!DB_.tables ().contains ("forms_never"))
				{
					if (!query.exec ("CREATE TABLE forms_never ("
								"url TEXT PRIMARY KEY"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return;
					}
				}
			}

			void SQLStorageBackendMysql::CheckVersions ()
			{
			}

			QString SQLStorageBackendMysql::GetSetting (const QString& key) const
			{
				QSqlQuery query (DB_);
				query.prepare ("SELECT value "
						"FROM storage_settings "
						"WHERE key = ? ");
				query.bindValue (0, key);
				if (!query.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (query);
					throw std::runtime_error ("SQLStorageBackendMysql could not query settings");
				}

				if (!query.next ())
					return QString ();

				return query.value (0).toString ();
			}

			void SQLStorageBackendMysql::SetSetting (const QString& key, const QString& value)
			{
				QSqlQuery query (DB_);
				QString r = "INSERT INTO storage_settings ("
							"key, "
							"value"
							") VALUES ("
							" ? , "
							" ? "
							")";
				query.prepare (r);
				query.bindValue (0, key);
				query.bindValue (1, value);
				if (!query.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (query);
					throw std::runtime_error ("SQLStorageBackendMysql could not query settings");
				}
			}
		};
	};
};

