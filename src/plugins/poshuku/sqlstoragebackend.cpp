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

#include "sqlstoragebackend.h"
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
			SQLStorageBackend::SQLStorageBackend (StorageBackend::Type type)
			: Type_ (type)
			{
				QString strType;
				switch (Type_)
				{
					case SBSQLite:
						strType = "QSQLITE";
						break;
					case SBPostgres:
						strType = "QPSQL";
				}

				DB_ = QSqlDatabase::addDatabase (strType, "PoshukuConnection");
				switch (Type_)
				{
					case SBSQLite:
						{
							QDir dir = QDir::home ();
							dir.cd (".leechcraft");
							dir.cd ("poshuku");
							DB_.setDatabaseName (dir.filePath ("poshuku.db"));
						}
						break;
					case SBPostgres:
						{
							DB_.setDatabaseName (XmlSettingsManager::Instance ()->
									property ("PostgresDBName").toString ());
							DB_.setHostName (XmlSettingsManager::Instance ()->
									property ("PostgresHostname").toString ());
							DB_.setPort (XmlSettingsManager::Instance ()->
									property ("PostgresPort").toInt ());
							DB_.setUserName (XmlSettingsManager::Instance ()->
									property ("PostgresUsername").toString ());
							DB_.setPassword (XmlSettingsManager::Instance ()->
									property ("PostgresPassword").toString ());
						}
						break;
				}

				if (!DB_.open ())
				{
					LeechCraft::Util::DBLock::DumpError (DB_.lastError ());
					throw std::runtime_error (QString ("Could not initialize database: %1")
							.arg (DB_.lastError ().text ()).toUtf8 ().constData ());
				}

				InitializeTables ();
				CheckVersions ();
			}

			SQLStorageBackend::~SQLStorageBackend ()
			{
				if (Type_ == SBSQLite &&
						XmlSettingsManager::Instance ()->property ("SQLiteVacuum").toBool ())
				{
					QSqlQuery vacuum (DB_);
					vacuum.exec ("VACUUM;");
				}
			}

			void SQLStorageBackend::Prepare ()
			{
				if (Type_ == SBSQLite)
				{
					QSqlQuery pragma (DB_);
					if (!pragma.exec (QString ("PRAGMA journal_mode = %1;")
								.arg (XmlSettingsManager::Instance ()->
									property ("SQLiteJournalMode").toString ())))
						LeechCraft::Util::DBLock::DumpError (pragma);
					if (!pragma.exec (QString ("PRAGMA synchronous = %1;")
								.arg (XmlSettingsManager::Instance ()->
									property ("SQLiteSynchronous").toString ())))
						LeechCraft::Util::DBLock::DumpError (pragma);
					if (!pragma.exec (QString ("PRAGMA temp_store = %1;")
								.arg (XmlSettingsManager::Instance ()->
									property ("SQLiteTempStore").toString ())))
						LeechCraft::Util::DBLock::DumpError (pragma);
				}

				HistoryLoader_ = QSqlQuery (DB_);
				HistoryLoader_.prepare ("SELECT "
						"title, "
						"date, "
						"url "
						"FROM history "
						"ORDER BY date DESC");

				HistoryRatedLoader_ = QSqlQuery (DB_);
				switch (Type_)
				{
					case SBSQLite:
						HistoryRatedLoader_.prepare ("SELECT "
								"SUM (julianday (date)) - julianday (MIN (date)) * COUNT (date) AS rating, "
								"title, "
								"url "
								"FROM history "
								"WHERE ( title LIKE :titlebase ) "
								"OR ( url LIKE :urlbase ) "
								"GROUP BY url "
								"ORDER BY rating DESC "
								"LIMIT 100");
						break;
					case SBPostgres:
						HistoryRatedLoader_.prepare ("SELECT "
								"SUM (AGE (date)) - AGE (MIN (date)) * COUNT (date) AS rating, "
								"MAX (title) AS title, "
								"url "
								"FROM history "
								"WHERE ( title LIKE :titlebase ) "
								"OR ( url LIKE :urlbase ) "
								"GROUP BY url "
								"ORDER BY rating ASC "
								"LIMIT 100");
						break;
				}

				HistoryAdder_ = QSqlQuery (DB_);
				HistoryAdder_.prepare ("INSERT INTO history ("
						"date, "
						"title, "
						"url"
						") VALUES ("
						":date, "
						":title, "
						":url"
						")");

				HistoryEraser_ = QSqlQuery (DB_);
				switch (Type_)
				{
					case SBSQLite:
						HistoryEraser_.prepare ("DELETE FROM history "
								"WHERE "
								"(julianday ('now') - julianday (date) > :age)");
						break;
					case SBPostgres:
						HistoryEraser_.prepare ("DELETE FROM history "
								"WHERE "
								"(date - now () > :age * interval '1 day')");
						break;
				}

				HistoryTruncater_ = QSqlQuery (DB_);
				switch (Type_)
				{
					case SBSQLite:
						HistoryTruncater_.prepare ("DELETE FROM history "
								"WHERE date IN "
								"(SELECT date FROM history ORDER BY date DESC "
								"LIMIT 10000 OFFSET :num)");
						break;
					case SBPostgres:
						HistoryTruncater_.prepare ("DELETE FROM history "
								"WHERE date IN "
								"	(SELECT date FROM history ORDER BY date DESC OFFSET :num)");
						break;
				}

				FavoritesLoader_ = QSqlQuery (DB_);
				switch (Type_)
				{
					case SBSQLite:
						FavoritesLoader_.prepare ("SELECT "
								"title, "
								"url, "
								"tags "
								"FROM favorites "
								"ORDER BY ROWID DESC");
						break;
					case SBPostgres:
						FavoritesLoader_.prepare ("SELECT "
								"title, "
								"url, "
								"tags "
								"FROM favorites "
								"ORDER BY CTID DESC");
						break;
				}

				FavoritesAdder_ = QSqlQuery (DB_);
				FavoritesAdder_.prepare ("INSERT INTO favorites ("
						"title, "
						"url, "
						"tags"
						") VALUES ("
						":title, "
						":url, "
						":tags"
						")");

				FavoritesUpdater_ = QSqlQuery (DB_);
				FavoritesUpdater_.prepare ("UPDATE favorites SET "
						"title = :title, "
						"tags = :tags "
						"WHERE url = :url");

				FavoritesRemover_ = QSqlQuery (DB_);
				FavoritesRemover_.prepare ("DELETE FROM favorites "
						"WHERE url = :url");

				FormsIgnoreSetter_ = QSqlQuery (DB_);
				FormsIgnoreSetter_.prepare ("INSERT INTO forms_never ("
						"url"
						") VALUES ("
						":url"
						")");

				FormsIgnoreGetter_ = QSqlQuery (DB_);
				FormsIgnoreGetter_.prepare ("SELECT COUNT (url) AS num "
						"FROM forms_never "
						"WHERE url = :url");

				FormsIgnoreClearer_ = QSqlQuery (DB_);
				FormsIgnoreClearer_.prepare ("DELETE FROM forms_never ("
						"WHERE url = :url");
			}

			void SQLStorageBackend::LoadHistory (history_items_t& items) const
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

			void SQLStorageBackend::LoadResemblingHistory (const QString& base,
					history_items_t& items) const
			{
				QString bound = "%";
				bound += base;
				bound += "%";
				HistoryRatedLoader_.bindValue (":titlebase", bound);
				HistoryRatedLoader_.bindValue (":urlbase", bound);
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

			void SQLStorageBackend::AddToHistory (const HistoryItem& item)
			{
				HistoryAdder_.bindValue (":title", item.Title_);
				HistoryAdder_.bindValue (":date", item.DateTime_);
				HistoryAdder_.bindValue (":url", item.URL_);

				if (!HistoryAdder_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (HistoryAdder_);
					return;
				}

				emit added (item);
			}

			void SQLStorageBackend::ClearOldHistory (int age, int items)
			{
				LeechCraft::Util::DBLock lock (DB_);
				lock.Init ();
				HistoryEraser_.bindValue (":age", age);
				HistoryTruncater_.bindValue (":num", items);

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

			void SQLStorageBackend::LoadFavorites (
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

			void SQLStorageBackend::AddToFavorites (const FavoritesModel::FavoritesItem& item)
			{
				FavoritesAdder_.bindValue (":title", item.Title_);
				FavoritesAdder_.bindValue (":url", item.URL_);
				FavoritesAdder_.bindValue (":tags", item.Tags_.join (" "));

				if (!FavoritesAdder_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FavoritesAdder_);
					throw std::runtime_error ("Failed to execute FavoritesAdder query.");
				}

				emit added (item);
			}

			void SQLStorageBackend::RemoveFromFavorites (const FavoritesModel::FavoritesItem& item)
			{
				FavoritesRemover_.bindValue (":url", item.URL_);
				if (!FavoritesRemover_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FavoritesRemover_);
					return;
				}

				emit removed (item);
			}

			void SQLStorageBackend::UpdateFavorites (const FavoritesModel::FavoritesItem& item)
			{
				FavoritesUpdater_.bindValue (":title", item.Title_);
				FavoritesUpdater_.bindValue (":url", item.URL_);
				FavoritesUpdater_.bindValue (":tags", item.Tags_.join (" "));

				if (!FavoritesUpdater_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FavoritesUpdater_);
					return;
				}

				emit updated (item);
			}

			void SQLStorageBackend::SetFormsIgnored (const QString& url, bool ignore)
			{
				if (ignore)
				{
					FormsIgnoreSetter_.bindValue (":url", url);
					if (!FormsIgnoreSetter_.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (FormsIgnoreSetter_);
						return;
					}
				}
				else
				{
					FormsIgnoreClearer_.bindValue (":url", url);
					if (!FormsIgnoreClearer_.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (FormsIgnoreClearer_);
						return;
					}
				}
			}

			bool SQLStorageBackend::GetFormsIgnored (const QString& url) const
			{
				FormsIgnoreGetter_.bindValue (":url", url);
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

			void SQLStorageBackend::InitializeTables ()
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

					if (Type_ == SBPostgres)
					{
						if (!query.exec ("CREATE RULE \"replace_storage_settings\" AS "
											"ON INSERT TO \"storage_settings\" "
											"WHERE "
												"EXISTS (SELECT 1 FROM storage_settings "
													"WHERE key = NEW.key) "
											"DO INSTEAD "
												"(UPDATE storage_settings "
													"SET value = NEW.value "
													"WHERE key = NEW.key)"))
						{
							LeechCraft::Util::DBLock::DumpError (query);
							return;
						}
					}

					SetSetting ("historyversion", "1");
					SetSetting ("favoritesversion", "1");
					SetSetting ("storagesettingsversion", "1");
				}

				if (!DB_.tables ().contains ("forms"))
				{
					QString binary = "BLOB";
					if (Type_ == SBPostgres)
						binary = "BYTEA";

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

			void SQLStorageBackend::CheckVersions ()
			{
			}

			QString SQLStorageBackend::GetSetting (const QString& key) const
			{
				QSqlQuery query (DB_);
				query.prepare ("SELECT value "
						"FROM storage_settings "
						"WHERE key = :key");
				query.bindValue (":key", key);
				if (!query.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (query);
					throw std::runtime_error ("SQLStorageBackend could not query settings");
				}

				if (!query.next ())
					return QString ();

				return query.value (0).toString ();
			}

			void SQLStorageBackend::SetSetting (const QString& key, const QString& value)
			{
				QSqlQuery query (DB_);
				QString r;
				switch (Type_)
				{
					case SBSQLite:
						r = "INSERT OR REPLACE INTO storage_settings ("
						"key, "
						"value"
						") VALUES ("
						":key, "
						":value"
						")";
						break;
					case SBPostgres:
						r = "INSERT INTO storage_settings ("
						"key, "
						"value"
						") VALUES ("
						":key, "
						":value"
						")";
						break;
				}
				query.prepare (r);
				query.bindValue (":key", key);
				query.bindValue (":value", value);
				if (!query.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (query);
					throw std::runtime_error ("SQLStorageBackend could not query settings");
				}
			}
		};
	};
};

