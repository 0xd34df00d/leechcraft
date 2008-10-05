#include "sqlstoragebackend.h"
#include <QDir>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

SQLStorageBackend::SQLStorageBackend ()
: DB_ (QSqlDatabase::addDatabase ("QSQLITE"))
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");

	DB_.setDatabaseName (dir.filePath ("aggregator.db"));
	if (!DB_.open ())
		DumpError (DB_.lastError ());

	if (!DB_.tables ().contains ("feeds"))
		InitializeTables ();
}

SQLStorageBackend::~SQLStorageBackend ()
{
	DB_.close ();
}

void SQLStorageBackend::GetFeeds (feeds_container_t& result) const
{
}

void SQLStorageBackend::GetChannels (const Feed_ptr& feed,
		channels_container_t& result) const
{
}

void SQLStorageBackend::GetItems (const Channel_ptr& channel,
		items_container_t& result) const
{
}

void SQLStorageBackend::AddFeed (const Feed_ptr& feed)
{
}

void SQLStorageBackend::UpdateFeed (const Feed_ptr& feed)
{
}

void SQLStorageBackend::UpdateItem (const Item_ptr& item)
{
}

void SQLStorageBackend::InitializeTables ()
{
	QSqlQuery query;
	query.exec ("CREATE TABLE feeds ("
			"url TEXT PRIMARY KEY, "
			"last_update TIMESTAMP "
			");");
	query.exec ("CREATE TABLE channels ("
			"parent_feed_url TEXT PRIMARY KEY, "
			"url TEXT, "
			"title TEXT, "
			"description TEXT, "
			"last_build TIMESTAMP, "
			"tags TEXT, "
			"language TEXT, "
			"author TEXT, "
			"pixmap_url TEXT, "
			"pixmap BLOB, "
			"favicon BLOB "
			");");
	query.exec ("CREATE TABLE items ("
			"parents_hash TEXT PRIMARY KEY, "
			"title TEXT, "
			"url TEXT, "
			"description TEXT, "
			"author TEXT, "
			"category TEXT, "
			"guid TEXT, "
			"pub_date TIMESTAMP, "
			"unread TINYINT "
			");");
	query.exec ("CREATE UNIQUE INDEX feeds_url ON feeds (url);");
	query.exec ("CREATE UNIQUE INDEX channels_parent_url ON channels (parent_feed_url);");
	query.exec ("CREATE UNIQUE INDEX items_parents_hash ON items (parents_hash);");
}

void SQLStorageBackend::DumpError (const QSqlError& lastError) const
{
	qDebug () << lastError.text () << "|"
		<< lastError.databaseText () << "|"
		<< lastError.driverText () << "|"
		<< lastError.type () << "|"
		<< lastError.number ();
}

