/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "accountstorage.h"
#include <stdexcept>
#include <QSqlError>
#include <QSqlRecord>
#include <util/dblock.h>
#include <util/util.h>
#include "localblogaccount.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	AccountStorage::AccountStorage (LocalBlogAccount *acc, QObject *parent)
	: QObject (parent)
	, Account_ (acc)
	, Ready_ (false)
	{
	}

	void AccountStorage::Init (const QString& dbPath)
	{
		if (AccountDB_.isValid ())
		{
			AccountDB_.close ();
			QSqlDatabase::removeDatabase (AccountDB_.connectionName ());
		}

		AccountDB_ = QSqlDatabase::addDatabase ("QSQLITE",
				QString (Account_->GetAccountID () + "_DataBase"));

		AccountDB_.setDatabaseName (dbPath);

		if (!AccountDB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (AccountDB_.lastError ());
			throw std::runtime_error ("unable to open account database");
		}

		{
			QSqlQuery query (AccountDB_);
			query.exec ("PRAGMA foreign_keys = ON;");
		}

		try
		{
			CreateTables ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		PrepareQueries ();
	}

	bool AccountStorage::IsReady () const
	{
		return Ready_;
	}

	bool AccountStorage::CheckDatabase (const QString& dbPath)
	{
		QSqlDatabase db = QSqlDatabase::addDatabase ("QSQLITE",
				QString ("Validating_DataBase"));
		db.setDatabaseName (dbPath);

		bool entriesTable = false;
		bool entryTagsTable = false;
		for (const auto& tableName : db.tables ())
		{
			QSqlRecord rec = db.record (tableName);
			if (tableName == "entries")
				entriesTable = rec.contains ("Id") &&
						rec.contains ("Entry") &&
						rec.contains ("Date") &&
						rec.contains ("Subject");
			else if (tableName == "entry_tags")
				entryTagsTable = rec.contains ("Id") &&
						rec.contains ("Tag") &&
						rec.contains ("EntryId");
		}

		return entriesTable && entryTagsTable;
	}

	void AccountStorage::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["entries"] = "CREATE TABLE IF NOT EXISTS drafts ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Entry TEXT, "
				"Date DATE, "
				"Subject TEXT "
				");";
		table2query ["entry_tags"] = "CREATE TABLE IF NOT EXISTS tags ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Tag TEXT NOT NULL, "
				"EntryId INTEGER NOT NULL REFERENCES entries (Id) ON DELETE CASCADE "
				");";

		Util::DBLock lock (AccountDB_);

		lock.Init ();

		const auto& tables = AccountDB_.tables ();
		Q_FOREACH (const QString& key, table2query.keys ())
		if (!tables.contains (key))
		{
			QSqlQuery q (AccountDB_);
			if (!q.exec (table2query [key]))
			{
				Util::DBLock::DumpError (q);
				throw std::runtime_error ("cannot create required tables");
			}
		}

		lock.Good ();
	}

	void AccountStorage::PrepareQueries ()
	{
		AddEntry_ = QSqlQuery (AccountDB_);
		AddEntry_.prepare ("INSERT OR REPLACE INTO entries (Entry, Date, Subject) "
				"VALUES (:entry_id, :entry, :date, :subject);");
		UpdateEntry_ = QSqlQuery (AccountDB_);
		UpdateEntry_.prepare ("UPDATE entries SET Entry = :entry, Date = :date, "
				"Subject = :subject WHERE Id = :entry_id;");
		RemoveEntry_ = QSqlQuery (AccountDB_);
		RemoveEntry_.prepare ("DELETE FROM entries WHERE Id = :entry_id;");

		GetEntries_ = QSqlQuery (AccountDB_);
		GetEntries_.prepare ("SELECT Id, EntryId,  Entry, Date, Subject FROM entries "
				"ORDER BY Date DESC;");
		GetEntriesByDate_= QSqlQuery (AccountDB_);
		GetEntriesByDate_.prepare ("SELECT Id, Entry, Date, Subject FROM entries "
				"WHERE date (Date) = :date;");
		GetEntriesCountByDate_ = QSqlQuery (AccountDB_);
		GetEntriesCountByDate_.prepare ("SELECT date (Date), COUNT (Id) FROM entries "
				"GROUP BY date (Date);");

		AddEntryTag_ = QSqlQuery (AccountDB_);
		AddEntryTag_.prepare ("INSERT INTO entry_tags "
				"(Tag, EntryID) VALUES (:tag, (SELECT Id FROM entries "
				"WHERE EntryId = :entry_id));");
		RemoveEntryTags_ = QSqlQuery (AccountDB_);
		RemoveEntryTags_.prepare ("DELETE FROM entry_tags WHERE EntryID = ("
				" SELECT Id FROM entries WHERE EntryId = :entry_id);");
		GetEntryTags_ = QSqlQuery (AccountDB_);
		GetEntryTags_.prepare ("SELECT Id, Tag FROM entry_tags WHERE EntryID = ("
				" SELECT Id FROM entries WHERE EntryId = :entry_id);");
	}

}
}
}