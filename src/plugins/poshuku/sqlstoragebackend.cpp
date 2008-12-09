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

	AuthGetter_ = QSqlQuery (DB_);
	AuthGetter_.prepare ("SELECT "
			"login, "
			"password "
			"FROM auth "
			"WHERE realm = :realm");

	AuthInserter_ = QSqlQuery (DB_);
	AuthInserter_.prepare ("INSERT INTO auth ("
			"realm, "
			"login, "
			"password"
			") VALUES ("
			":realm, "
			":login, "
			":password"
			")");

	AuthUpdater_ = QSqlQuery (DB_);
	AuthUpdater_.prepare ("UPDATE auth SET "
			"login = :login, "
			"password = :password "
			"WHERE realm = :realm");
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

void SQLStorageBackend::AddToHistory (const HistoryModel::HistoryItem& item)
{
	HistoryAdder_.bindValue (":title", item.Title_);
	HistoryAdder_.bindValue (":date", item.DateTime_);
	HistoryAdder_.bindValue (":url", item.URL_);

	if (!HistoryAdder_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (HistoryLoader_);
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

void SQLStorageBackend::GetAuth (const QString& realm,
		QString& login, QString& password) const
{
	AuthGetter_.bindValue (":realm", realm);

	if (!AuthGetter_.exec () || !AuthGetter_.next ())
	{
		LeechCraft::Util::DBLock::DumpError (AuthGetter_);
		return;
	}

	login = AuthGetter_.value (0).toString ();
	password = AuthGetter_.value (1).toString ();
}

void SQLStorageBackend::SetAuth (const QString& realm,
		const QString& login, const QString& password)
{
	AuthGetter_.bindValue (":realm", realm);

	if (!AuthGetter_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (AuthGetter_);
		return;
	}

	if (!AuthGetter_.size ())
	{
		AuthInserter_.bindValue (":realm", realm);
		AuthInserter_.bindValue (":login", login);
		AuthInserter_.bindValue (":password", password);
		if (!AuthInserter_.exec ())
		{
			LeechCraft::Util::DBLock::DumpError (AuthInserter_);
			return;
		}
	}
	else
	{
		AuthUpdater_.bindValue (":realm", realm);
		AuthUpdater_.bindValue (":login", login);
		AuthUpdater_.bindValue (":password", password);
		if (!AuthUpdater_.exec ())
		{
			LeechCraft::Util::DBLock::DumpError (AuthUpdater_);
			return;
		}
	}
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

	if (!query.exec ("CREATE TABLE auth ("
				"realm TEXT PRIMARY KEY, "
				"login TEXT, "
				"password TEXT"
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
	if (!query.exec ("CREATE UNIQUE INDEX auth_realm "
				"ON auth (realm);"))
		LeechCraft::Util::DBLock::DumpError (query);
}

