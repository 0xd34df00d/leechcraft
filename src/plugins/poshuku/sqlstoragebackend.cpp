#include "sqlstoragebackend.h"
#include <stdexcept>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>
#include <plugininterface/dblock.h>
#include "xmlsettingsmanager.h"

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
		throw std::runtime_error ("Could not initialize database");
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
					"(age (date) > :age)");
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

	ThumbnailsGetter_ = QSqlQuery (DB_);
	ThumbnailsGetter_.prepare ("SELECT "
			"shot_date, "
			"res_x, "
			"res_y, "
			"thumbnail "
			"FROM thumbnails "
			"WHERE url = :url");

	ThumbnailsSetter_ = QSqlQuery (DB_);
	ThumbnailsSetter_.prepare ("INSERT OR REPLACE INTO thumbnails ("
			"url, "
			"shot_date, "
			"res_x, "
			"res_y, "
			"thumbnail"
			") VALUES ("
			":url, "
			":shot_date, "
			":res_x, "
			":res_y, "
			":thumbnail"
			")");
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

void SQLStorageBackend::ClearOldHistory (int age)
{
	LeechCraft::Util::DBLock lock (DB_);
	lock.Init ();
	HistoryEraser_.bindValue (":age", age);

	if (HistoryEraser_.exec ())
		lock.Good ();
	else
		LeechCraft::Util::DBLock::DumpError (HistoryEraser_);
}

void SQLStorageBackend::LoadFavorites (
		std::vector<FavoritesModel::FavoritesItem>& items
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

void SQLStorageBackend::GetThumbnail (SpeedDialProvider::Item& item) const
{
	ThumbnailsGetter_.bindValue (":url", item.URL_);
	if (!ThumbnailsGetter_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (ThumbnailsGetter_);
		return;
	}

	if (ThumbnailsGetter_.next ())
	{
		item.ShotDate_ = ThumbnailsGetter_.value (0).toDateTime ();
		item.ResX_ = ThumbnailsGetter_.value (1).toInt ();
		item.ResY_ = ThumbnailsGetter_.value (2).toInt ();
		item.Thumb_ = ThumbnailsGetter_.value (3).toByteArray ();
	}

	ThumbnailsGetter_.finish ();
}

void SQLStorageBackend::SetThumbnail (const SpeedDialProvider::Item& item)
{
	ThumbnailsSetter_.bindValue (":url", item.URL_);
	ThumbnailsSetter_.bindValue (":shot_date", item.ShotDate_);
	ThumbnailsSetter_.bindValue (":res_x", item.ResX_);
	ThumbnailsSetter_.bindValue (":res_y", item.ResY_);
	ThumbnailsSetter_.bindValue (":thumbnail", item.Thumb_);

	if (!ThumbnailsSetter_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (ThumbnailsSetter_);
		return;
	}
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

	if (!DB_.tables ().contains ("thumbnails"))
	{
		QString request;
		switch (Type_)
		{
			case SBSQLite:
				request = "CREATE TABLE thumbnails ("
					"url TEXT PRIMARY KEY, "
					"shot_date TIMESTAMP, "
					"res_x INTEGER, "
					"res_y INTEGER, "
					"thumbnail BLOB"
					");";
				break;
			case SBPostgres:
				request = "CREATE TABLE thumbnails ("
					"url TEXT PRIMARY KEY, "
					"shot_date TIMESTAMP, "
					"res_x INTEGER, "
					"res_y INTEGER, "
					"thumbnail BYTEA"
					");";
				break;
		}
		if (!query.exec (request))
		{
			LeechCraft::Util::DBLock::DumpError (query);
			return;
		}

		SetSetting ("thumbnailsversion", "1");
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
		throw std::runtime_error ("No such field");

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

