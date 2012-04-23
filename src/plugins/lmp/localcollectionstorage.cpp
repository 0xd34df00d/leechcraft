/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "localcollectionstorage.h"
#include <stdexcept>
#include <QSqlError>
#include <QSqlQuery>
#include <util/util.h>
#include <util/dblock.h>

namespace LeechCraft
{
namespace LMP
{
	LocalCollectionStorage::LocalCollectionStorage (QObject *parent)
	: QObject (parent)
	, DB_ (QSqlDatabase::addDatabase ("QSQLITE", "LMP_LocalCollection"))
	{
		DB_.setDatabaseName (Util::CreateIfNotExists ("lmp").filePath ("localcollection.db"));

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error ("unable to open Azoth history database");
		}

		{
			QSqlQuery query (DB_);
			query.exec ("PRAGMA foreign_keys = ON;");
			query.exec ("PRAGMA synchronous = OFF;");
		}

		CreateTables ();
	}

	void LocalCollectionStorage::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["artists"] = "CREATE TABLE artists ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Name TEXT "
				");";
		table2query ["albums"] = "CREATE TABLE albums ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Name TEXT, "
				"Year INTEGER, "
				"CoverImage TEXT "
				");";
		table2query ["artists2albums"] = "CREATE TABLE artists2albums ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"ArtistID INTEGER NOT NULL REFERENCES artists (Id) ON DELETE CASCADE, "
				"AlbumID INTEGER NOT NULL REFERENCES albums (Id) ON DELETE CASCADE "
				");";
		table2query ["tracks"] = "CREATE TABLE tracks ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"ArtistID INTEGER NOT NULL REFERENCES artists (Id) ON DELETE CASCADE, "
				"AlbumId NOT NULL REFERENCES albums (Id) ON DELETE CASCADE, "
				"Path TEXT NOT NULL, "
				"Name TEXT NOT NULL, "
				"TrackNumber INTEGER, "
				"Length INTEGER "
				");";
		table2query ["genres"] = "CREATE TABLE genres ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"Name TEXT NOT NULL "
				");";
		table2query ["statistics"] = "CREATE TABLE statistics ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"Playcount INTEGER, "
				"Added TIMESTAMP, "
				"LastPlay TIMESTAMP, "
				"Score INTEGER, "
				"Rating INTEGER "
				");";

		Util::DBLock lock (DB_);

		lock.Init ();

		const auto& tables = DB_.tables ();
		Q_FOREACH (const QString& key, table2query.keys ())
			if (!tables.contains (key))
			{
				QSqlQuery q;
				if (!q.exec (table2query [key]))
				{
					Util::DBLock::DumpError (q);
					throw std::runtime_error ("LMP: cannot create required tables");
				}
			}

		lock.Good ();
	}
}
}
