/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountstorage.h"
#include <stdexcept>
#include <QSqlError>
#include <QSqlRecord>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/sll/qtutil.h>
#include <util/util.h>
#include "localblogaccount.h"

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	AccountStorage::AccountStorage (LocalBlogAccount *acc, QObject *parent)
	: QObject (parent)
	, Account_ (acc)
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
			query.exec ("PRAGMA synchronous = OFF;");
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

	bool AccountStorage::CheckDatabase (const QString& dbPath)
	{
		QSqlDatabase db = QSqlDatabase::addDatabase ("QSQLITE",
				QString ("Validating_DataBase_%1")
					.arg (QString::fromUtf8 (Account_->GetAccountID ())));
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

	qint64 AccountStorage::SaveNewEntry (const Entry& e)
	{
		Util::DBLock lock (AccountDB_);
		lock.Init ();

		AddEntry_.bindValue (":entry", e.Content_);
		AddEntry_.bindValue (":date", e.Date_);
		AddEntry_.bindValue (":subject", e.Subject_);
		if (!AddEntry_.exec ())
		{
			Util::DBLock::DumpError (AddEntry_);
			throw std::runtime_error ("unable to add entry");
		}

		const qint64 id = AddEntry_.lastInsertId ().toLongLong ();

		for (const auto& tag : e.Tags_)
		{
			if (tag.isEmpty ())
				continue;

			AddEntryTag_.bindValue (":tag", tag);
			AddEntryTag_.bindValue (":entry_id", id);
			if (!AddEntryTag_.exec ())
			{
				Util::DBLock::DumpError (AddEntryTag_);
				throw std::runtime_error ("unable to add entry's tag");
			}
		}

		lock.Good ();
		return id;
	}

	qint64 AccountStorage::UpdateEntry (const Entry& e, qint64 entryId)
	{
		Util::DBLock lock (AccountDB_);
		lock.Init ();

		Entry entry = GetFullEntry (entryId);
		if (entry.IsEmpty ())
			entryId = SaveNewEntry (e);
		else
		{
			UpdateEntry_.bindValue (":entry", e.Content_);
			UpdateEntry_.bindValue (":date", e.Date_);
			UpdateEntry_.bindValue (":subject", e.Subject_);
			UpdateEntry_.bindValue (":draft_id", entryId);
			if (!UpdateEntry_.exec ())
			{
				Util::DBLock::DumpError (UpdateEntry_);
				throw std::runtime_error ("unable to update entry");
			}
		}

		RemoveEntryTags_.bindValue (":entry_id", entryId);
		if (!RemoveEntryTags_.exec ())
		{
			Util::DBLock::DumpError (RemoveEntryTags_);
			throw std::runtime_error ("unable to remove entry's tags");
		}

		for (const auto& tag : e.Tags_)
		{
			if (tag.isEmpty ())
				continue;

			AddEntryTag_.bindValue (":tag", tag);
			AddEntryTag_.bindValue (":entry_id", entryId);
			if (!AddEntryTag_.exec ())
			{
				Util::DBLock::DumpError (AddEntryTag_);
				throw std::runtime_error ("unable to add entry's tag");
			}
		}

		lock.Good ();

		return entryId;
	}

	void AccountStorage::RemoveEntry (qint64 entryId)
	{
		Util::DBLock lock (AccountDB_);
		lock.Init ();

		RemoveEntry_.bindValue (":entry_id", entryId);
		if (!RemoveEntry_.exec ())
		{
			Util::DBLock::DumpError (RemoveEntry_);
			throw std::runtime_error ("unable to remove entry");
		}

		lock.Good ();
	}

	namespace
	{
		QStringList GetTags (QSqlQuery& query)
		{
			if (!query.exec ())
			{
				Util::DBLock::DumpError (query);
				throw std::runtime_error ("unable to get entry's tag");
			}

			QStringList tags;
			while (query.next ())
				tags << query.value (1).toString ();

			query.finish ();

			return tags;
		}
	}

	QList<Entry> AccountStorage::GetEntries (AccountStorage::Mode mode)
	{
		Q_UNUSED (mode);

		if (!GetEntries_.exec ())
		{
			Util::DBLock::DumpError (GetEntries_);
			throw std::runtime_error ("unable to get entries");
		}

		QList<Entry> list;
		while (GetEntries_.next ())
		{
			Entry e;
			e.EntryId_ = GetEntries_.value (0).toLongLong ();
			e.Content_ = GetEntries_.value (1).toString ();
			e.Date_ = GetEntries_.value (2).toDateTime ();
			e.Subject_ = GetEntries_.value (3).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);

			list << e;
		}
		GetEntries_.finish ();

		return list;
	}

	QList<Entry> AccountStorage::GetLastEntries (AccountStorage::Mode mode, int count)
	{
		Q_UNUSED (mode);

		GetLastEntries_.bindValue (":limit", count);
		if (!GetLastEntries_.exec ())
		{
			Util::DBLock::DumpError (GetLastEntries_);
			throw std::runtime_error ("unable to get entries");
		}

		QList<Entry> list;
		while (GetLastEntries_.next ())
		{
			Entry e;
			e.EntryId_ = GetLastEntries_.value (0).toLongLong ();
			e.Content_ = GetLastEntries_.value (1).toString ();
			e.Date_ = GetLastEntries_.value (2).toDateTime ();
			e.Subject_ = GetLastEntries_.value (3).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);

			list << e;
		}
		GetLastEntries_.finish ();

		return list;
	}

	QList<Entry> AccountStorage::GetEntriesByDate (const QDate& date)
	{
		GetEntriesByDate_.bindValue (":date", date);
		if (!GetEntriesByDate_.exec ())
		{
			Util::DBLock::DumpError (GetEntriesByDate_);
			throw std::runtime_error ("unable to get entries");
		}

		QList<Entry> list;
		while (GetEntriesByDate_.next ())
		{
			Entry e;
			e.EntryId_ = GetEntriesByDate_.value (0).toInt ();
			e.Content_ = GetEntriesByDate_.value (1).toString ();
			e.Date_ = GetEntriesByDate_.value (2).toDateTime ();
			e.Subject_ = GetEntriesByDate_.value (3).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);

			list << e;
		}
		GetEntriesByDate_.finish ();

		return list;
	}

	QList<Entry> AccountStorage::GetEntriesWithFilter (const Filter& filter)
	{
		GetFilteredEntries_.bindValue (":begin_date", filter.BeginDate_);
		GetFilteredEntries_.bindValue (":end_date", filter.EndDate_);
		if (!GetFilteredEntries_.exec ())
		{
			Util::DBLock::DumpError (GetFilteredEntries_);
			throw std::runtime_error ("unable to get entries");
		}

		QList<Entry> list;
		while (GetFilteredEntries_.next ())
		{
			Entry e;
			e.EntryId_ = GetFilteredEntries_.value (0).toLongLong ();
			e.Content_ = GetFilteredEntries_.value (1).toString ();
			e.Date_ = GetFilteredEntries_.value (2).toDateTime ();
			e.Subject_ = GetFilteredEntries_.value (3).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);

			bool found = false;
			for (const auto& tag : filter.Tags_)
				if (e.Tags_.contains (tag))
				{
					found = true;
					break;
				}

			if (found)
				list << e;
		}
		GetFilteredEntries_.finish ();

		return list;
	}

	QMap<QDate, int> AccountStorage::GetEntriesCountByDate ()
	{
		if (!GetEntriesCountByDate_.exec ())
		{
			Util::DBLock::DumpError (GetEntriesCountByDate_);
			throw std::runtime_error ("unable to get entries");
		}

		QMap<QDate, int> statistic;
		while (GetEntriesCountByDate_.next ())
			statistic.insert (GetEntriesCountByDate_.value (0).toDate (),
					GetEntriesCountByDate_.value (1).toInt ());
		GetEntriesCountByDate_.finish ();

		return statistic;
	}

	Entry AccountStorage::GetFullEntry (qint64 entryId)
	{
		GetFullEntry_.bindValue (":entry_id", entryId);
		if (!GetFullEntry_.exec ())
		{
			Util::DBLock::DumpError (GetFullEntry_);
			throw std::runtime_error ("unable to get full entry by id");
		}

		Entry e;
		while (GetFullEntry_.next ())
		{
			e.EntryId_ = entryId;
			e.Content_ = GetFullEntry_.value (1).toString ();
			e.Date_ = GetFullEntry_.value (2).toDateTime ();
			e.Subject_ = GetFullEntry_.value (3).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);
		}
		GetFullEntry_.finish ();

		return e;
	}

	QHash<QString, int> AccountStorage::GetAllTags ()
	{
		if (!GetTags_.exec ())
		{
			Util::DBLock::DumpError (GetTags_);
			throw std::runtime_error ("unable to get tags");
		}

		QHash<QString, int> tags;
		while (GetTags_.next ())
			tags [GetTags_.value (0).toString ()] = GetTags_.value (1).toInt ();

		GetTags_.finish ();

		return tags;
	}

	void AccountStorage::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["entries"] = "CREATE TABLE IF NOT EXISTS entries ("
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
		for (const auto& [key, query] : Util::Stlize (table2query))
			if (!tables.contains (key))
			{
				QSqlQuery q (AccountDB_);
				if (!q.exec (query))
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
				"VALUES (:entry, :date, :subject);");
		UpdateEntry_ = QSqlQuery (AccountDB_);
		UpdateEntry_.prepare ("UPDATE entries SET Entry = :entry, Date = :date, "
				"Subject = :subject WHERE Id = :entry_id;");
		RemoveEntry_ = QSqlQuery (AccountDB_);
		RemoveEntry_.prepare ("DELETE FROM entries WHERE Id = :entry_id;");

		GetFullEntry_ = QSqlQuery (AccountDB_);
		GetFullEntry_.prepare ("SELECT Id, Entry, Date, Subject FROM entries "
				"WHERE Id = :entry_id");
		GetEntries_ = QSqlQuery (AccountDB_);
		GetEntries_.prepare ("SELECT Id, Entry, Date, Subject FROM entries "
				"ORDER BY Date DESC;");
		GetLastEntries_ = QSqlQuery (AccountDB_);
		GetLastEntries_.prepare ("SELECT Id, Entry, Date, Subject FROM entries "
				"ORDER BY Date DESC LIMIT :limit;");
		GetEntriesByDate_= QSqlQuery (AccountDB_);
		GetEntriesByDate_.prepare ("SELECT Id, Entry, Date, Subject FROM entries "
				"WHERE date (Date) = :date;");
		GetEntriesCountByDate_ = QSqlQuery (AccountDB_);
		GetEntriesCountByDate_.prepare ("SELECT date (Date), COUNT (Id) FROM entries "
				"GROUP BY date (Date);");
		GetFilteredEntries_ = QSqlQuery (AccountDB_);
		GetFilteredEntries_.prepare ("SELECT  Id, Entry, Date, Subject FROM entries "
				"WHERE Date >= :begin_date AND Date <= :end_date;");

		AddEntryTag_ = QSqlQuery (AccountDB_);
		AddEntryTag_.prepare ("INSERT INTO tags "
				"(Tag, EntryID) VALUES (:tag, :entry_id);");
		RemoveEntryTags_ = QSqlQuery (AccountDB_);
		RemoveEntryTags_.prepare ("DELETE FROM tags WHERE EntryID = ("
				" SELECT Id FROM entries WHERE EntryId = :entry_id);");
		GetEntryTags_ = QSqlQuery (AccountDB_);
		GetEntryTags_.prepare ("SELECT Id, Tag FROM tags WHERE EntryID = ("
				" SELECT Id FROM entries WHERE EntryId = :entry_id);");

		GetTags_ = QSqlQuery (AccountDB_);
		GetTags_.prepare ("SELECT Tag, COUNT (Tag) FROM tags GROUP BY Tag;");
	}

}
}
}
