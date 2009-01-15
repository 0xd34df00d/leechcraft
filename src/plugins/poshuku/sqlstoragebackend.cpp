#include "sqlstoragebackend.h"
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <plugininterface/dblock.h>

SQLStorageBackend::SQLStorageBackend ()
: DB_ (QSqlDatabase::addDatabase ("QSQLITE", "PoshukuConnection"))
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	dir.cd ("poshuku");
	DB_.setDatabaseName (dir.filePath ("poshuku.db"));
	if (!DB_.open ())
		LeechCraft::Util::DBLock::DumpError (DB_.lastError ());

	if (!DB_.tables ().contains ("history"))
		InitializeTables ();
}

SQLStorageBackend::~SQLStorageBackend ()
{
}

void SQLStorageBackend::Prepare ()
{
	QSqlQuery pragma (DB_);
	if (!pragma.exec ("PRAGMA journal_mode = TRUNCATE;"))
		LeechCraft::Util::DBLock::DumpError (pragma);
	if (!pragma.exec ("PRAGMA synchronous = OFF;"))
		LeechCraft::Util::DBLock::DumpError (pragma);
	if (!pragma.exec ("PRAGMA temp_store = MEMORY;"))
		LeechCraft::Util::DBLock::DumpError (pragma);

	HistoryLoader_ = QSqlQuery (DB_);
	HistoryLoader_.prepare ("SELECT "
			"title, "
			"date, "
			"url "
			"FROM history "
			"ORDER BY date DESC");

	HistoryUniqueLoader_ = QSqlQuery (DB_);
	HistoryUniqueLoader_.prepare ("SELECT "
			"title, "
			"url "
			"FROM history "
			"GROUP BY url");

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

	FavoritesLoader_ = QSqlQuery (DB_);
	FavoritesLoader_.prepare ("SELECT "
			"title, "
			"url, "
			"tags "
			"FROM favorites "
			"ORDER BY title");

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
}

void SQLStorageBackend::LoadHistory (
		std::vector<HistoryModel::HistoryItem>& items
		) const
{
	if (!HistoryLoader_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (HistoryLoader_);
		return;
	}

	while (HistoryLoader_.next ())
	{
		HistoryModel::HistoryItem item =
		{
			HistoryLoader_.value (0).toString (),
			HistoryLoader_.value (1).toDateTime (),
			HistoryLoader_.value (2).toString ()
		};
		items.push_back (item);
	}
}

void SQLStorageBackend::LoadUniqueHistory (
		std::vector<HistoryModel::HistoryItem>& items
		) const
{
	if (!HistoryUniqueLoader_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (HistoryUniqueLoader_);
		return;
	}

	while (HistoryUniqueLoader_.next ())
	{
		HistoryModel::HistoryItem item =
		{
			HistoryUniqueLoader_.value (0).toString (),
			QDateTime (),
			HistoryUniqueLoader_.value (1).toString ()
		};
		items.push_back (item);
	}
}

void SQLStorageBackend::AddToHistory (const HistoryModel::HistoryItem& item)
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
		return;
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

void SQLStorageBackend::InitializeTables ()
{
	QSqlQuery query (DB_);

	if (!query.exec ("CREATE TABLE history ("
				"date TIMESTAMP PRIMARY KEY, "
				"title TEXT, "
				"url TEXT"
				");"))
	{
		LeechCraft::Util::DBLock::DumpError (query);
		return;
	}

	if (!query.exec ("CREATE TABLE favorites ("
				"title TEXT PRIMARY KEY, "
				"url TEXT, "
				"tags TEXT"
				");"))
	{
		LeechCraft::Util::DBLock::DumpError (query);
		return;
	}

	if (!query.exec ("CREATE UNIQUE INDEX history_date "
				"ON history (date);"))
		LeechCraft::Util::DBLock::DumpError (query);
	if (!query.exec ("CREATE UNIQUE INDEX favorites_date "
				"ON favorites (title);"))
		LeechCraft::Util::DBLock::DumpError (query);
}

