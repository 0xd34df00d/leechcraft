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
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QtDebug>
#include <plugininterface/dblock.h>

using namespace LeechCraft;

SQLStorageBackend::SQLStorageBackend ()
: DB_ (QSqlDatabase::addDatabase ("QSQLITE", "CoreConnection"))
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	dir.cd ("core");
	DB_.setDatabaseName (dir.filePath ("core.db"));
	if (!DB_.open ())
		LeechCraft::Util::DBLock::DumpError (DB_.lastError ());

	if (!DB_.tables ().contains ("sitesAuth"))
		InitializeTables ();
}

SQLStorageBackend::~SQLStorageBackend ()
{
	AuthGetter_.finish ();
	AuthInserter_.finish ();
	AuthUpdater_.finish ();
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

	AuthGetter_ = QSqlQuery (DB_);
	AuthGetter_.prepare ("SELECT "
			"login, "
			"password "
			"FROM sitesAuth "
			"WHERE realm = :realm");

	AuthInserter_ = QSqlQuery (DB_);
	AuthInserter_.prepare ("INSERT INTO sitesAuth ("
			"realm, "
			"login, "
			"password"
			") VALUES ("
			":realm, "
			":login, "
			":password"
			")");

	AuthUpdater_ = QSqlQuery (DB_);
	AuthUpdater_.prepare ("UPDATE sitesAuth SET "
			"login = :login, "
			"password = :password "
			"WHERE realm = :realm");
}

void SQLStorageBackend::GetAuth (const QString& realm,
		QString& login, QString& password) const
{
	AuthGetter_.bindValue (":realm", realm);

	if (!AuthGetter_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (AuthGetter_);
		return;
	}
	if (!AuthGetter_.next ())
	{
		AuthGetter_.finish ();
		return;
	}

	login = AuthGetter_.value (0).toString ();
	password = AuthGetter_.value (1).toString ();
	AuthGetter_.finish ();
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

	AuthGetter_.finish ();

	if (AuthGetter_.size () <= 0)
	{
		AuthInserter_.bindValue (":realm", realm);
		AuthInserter_.bindValue (":login", login);
		AuthInserter_.bindValue (":password", password);
		if (!AuthInserter_.exec ())
		{
			LeechCraft::Util::DBLock::DumpError (AuthInserter_);
			return;
		}
		AuthInserter_.finish ();
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
		AuthUpdater_.finish ();
	}
}

void SQLStorageBackend::InitializeTables ()
{
	QSqlQuery query (DB_);

	if (!query.exec ("CREATE TABLE sitesAuth ("
				"realm TEXT PRIMARY KEY, "
				"login TEXT, "
				"password TEXT"
				");"))
	{
		LeechCraft::Util::DBLock::DumpError (query);
		return;
	}

	if (!query.exec ("CREATE UNIQUE INDEX sitesAuth_realm "
				"ON sitesAuth (realm);"))
		LeechCraft::Util::DBLock::DumpError (query);
}


