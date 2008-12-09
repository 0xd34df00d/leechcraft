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
			"url "
			") VALUES ("
			":date, "
			":title, "
			":url"
			")");
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

