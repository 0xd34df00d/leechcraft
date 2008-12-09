#include "sqlstoragebackend.h"
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <plugininterface/dblock.h>

SQLStorageBackend::SQLStorageBackend ()
: DB_ (QSqlDatabase::addDatabase ("QSQLITE"))
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

void SQLStorageBackend::InitializeTables ()
{
	QSqlQuery query;

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

