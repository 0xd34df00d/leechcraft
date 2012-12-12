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

#include "localstorage.h"
#include <stdexcept>
#include <QSqlError>
#include <util/util.h>
#include <util/dblock.h>

namespace LeechCraft
{
namespace Blogique
{
	LocalStorage::LocalStorage (QObject *obj)
	: QObject (obj)
	, DB_ (QSqlDatabase::addDatabase ("QSQLITE",
			QString ("Blogique_LocalStorage")))
	{
		DB_.setDatabaseName (Util::CreateIfNotExists ("blogique")
				.filePath ("localstorage.db"));

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error ("unable to open Blogique local storage database");
		}

		{
			QSqlQuery query (DB_);
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

	void LocalStorage::AddAccount (const QByteArray& accounId)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		AddAccount_.bindValue (":account_id", QString::fromUtf8 (accounId));
		if (!AddAccount_.exec ())
		{
			Util::DBLock::DumpError (AddAccount_);
			throw std::runtime_error ("unable to add account");
		}

		lock.Good ();
	}

	qlonglong LocalStorage::SaveDraft (const QByteArray& accountID, const Entry& e)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		AddDraft_.bindValue (":account_id", QString::fromUtf8 (accountID));
		AddDraft_.bindValue (":entry", e.Content_);
		AddDraft_.bindValue (":date", e.Date_);
		AddDraft_.bindValue (":subject", e.Subject_);
		if (!AddDraft_.exec ())
		{
			Util::DBLock::DumpError (AddDraft_);
			throw std::runtime_error ("unable to add draft");
		}

		const int id = AddDraft_.lastInsertId ().toInt ();

		RemoveDraftTags_.bindValue (":draft_id", id);
		if (!RemoveDraftTags_.exec ())
		{
			Util::DBLock::DumpError (RemoveDraftTags_);
			throw std::runtime_error ("unable to remove draft's tags");
		}

		for (const auto& tag : e.Tags_)
		{
			if (tag.isEmpty ())
				continue;

			AddDraftTag_.bindValue (":tag", tag);
			AddDraftTag_.bindValue (":draft_id", id);
			if (!AddDraftTag_.exec ())
			{
				Util::DBLock::DumpError (AddDraftTag_);
				throw std::runtime_error ("unable to add draft's tag");
			}
		}

		for (const auto& key : e.PostOptions_.keys ())
		{
			AddDraftPostOptions_.bindValue (":draft_id", id);
			AddDraftPostOptions_.bindValue (":name", key);
			AddDraftPostOptions_.bindValue (":value", e.PostOptions_.value (key));
			if (!AddDraftPostOptions_.exec ())
			{
				Util::DBLock::DumpError (AddDraftPostOptions_);
				throw std::runtime_error ("unable to add draft params");
			}
		}

		for (const auto& key : e.CustomData_.keys ())
		{
			AddDraftCustomOptions_.bindValue (":draft_id", id);
			AddDraftCustomOptions_.bindValue (":name", key);
			AddDraftCustomOptions_.bindValue (":value", e.CustomData_.value (key));
			if (!AddDraftCustomOptions_.exec ())
			{
				Util::DBLock::DumpError (AddDraftCustomOptions_);
				throw std::runtime_error ("unable to add draft params");
			}
		}

		lock.Good ();
		return id;
	}

	void LocalStorage::UpdateDraft (qlonglong id, const Entry& e)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		UpdateDraft_.bindValue (":entry", e.Content_);
		UpdateDraft_.bindValue (":date", e.Date_);
		UpdateDraft_.bindValue (":subject", e.Subject_);
		UpdateDraft_.bindValue (":draft_id", id);
		if (!UpdateDraft_.exec ())
		{
			Util::DBLock::DumpError (UpdateDraft_);
			throw std::runtime_error ("unable to update draft");
		}

		RemoveDraftTags_.bindValue (":draft_id", id);
		if (!RemoveDraftTags_.exec ())
		{
			Util::DBLock::DumpError (RemoveDraftTags_);
			throw std::runtime_error ("unable to remove draft's tags");
		}

		for (const auto& tag : e.Tags_)
		{
			if (tag.isEmpty ())
				continue;

			AddDraftTag_.bindValue (":tag", tag);
			AddDraftTag_.bindValue (":draft_id", id);
			if (!AddDraftTag_.exec ())
			{
				Util::DBLock::DumpError (AddDraftTag_);
				throw std::runtime_error ("unable to add draft's tag");
			}
		}

		for (const auto& key : e.PostOptions_.keys ())
		{
			UpdateDraftPostOptions_.bindValue (":draft_id", id);
			UpdateDraftPostOptions_.bindValue (":name", key);
			UpdateDraftPostOptions_.bindValue (":value", e.PostOptions_.value (key));
			if (!UpdateDraftPostOptions_.exec ())
			{
				Util::DBLock::DumpError (UpdateDraftPostOptions_);
				throw std::runtime_error ("unable to update draft post option");
			}
		}

		for (const auto& key : e.CustomData_.keys ())
		{
			UpdateDraftCustomOptions_.bindValue (":draft_id", id);
			UpdateDraftCustomOptions_.bindValue (":name", key);
			UpdateDraftCustomOptions_.bindValue (":value", e.PostOptions_.value (key));
			if (!UpdateDraftCustomOptions_.exec ())
			{
				Util::DBLock::DumpError (UpdateDraftCustomOptions_);
				throw std::runtime_error ("unable to update draft custom option");
			}
		}

		lock.Good ();
	}

	namespace
	{
		QVariantMap GetEntryOptions (QSqlQuery query)
		{
			if (!query.exec ())
			{
				Util::DBLock::DumpError (query);
				throw std::runtime_error ("unable to get entry's params");
			}

			QVariantMap map;
			while (query.next ())
				map.insert (query.value (1).toString (), query.value (2));
			query.finish ();

			return map;
		}

		QStringList GetTags (QSqlQuery query)
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

	QList<Entry> LocalStorage::GetDrafts (const QByteArray& accountId)
	{
		GetDrafts_.bindValue (":account_id", QString::fromUtf8 (accountId));
		if (!GetDrafts_.exec ())
		{
			Util::DBLock::DumpError (GetDrafts_);
			throw std::runtime_error ("unable to get drafts");
		}

		QList<Entry> list;
		while (GetDrafts_.next ())
		{
			Entry e;
			const int draftId = GetDrafts_.value (0).toInt ();
			e.EntryId_ = draftId;
			e.Content_ = GetDrafts_.value (1).toString ();
			e.Date_ = GetDrafts_.value (2).toDateTime ();
			e.Subject_ = GetDrafts_.value (3).toString ();

			GetDraftTags_.bindValue (":draft_id", draftId);
			e.Tags_ = GetTags (GetDraftTags_);

			GetDraftPostOptions_.bindValue (":draft_id", draftId);
			e.PostOptions_ = GetEntryOptions (GetDraftPostOptions_);

			GetDraftCustomOptions_.bindValue (":draft_id", draftId);
			e.CustomData_ = GetEntryOptions (GetDraftCustomOptions_);

			list << e;
		}
		GetDrafts_.finish ();

		return list;
	}

	Entry LocalStorage::GetFullDraft (const QByteArray& accountId, qlonglong draftId)
	{
		GetFullDraft_.bindValue (":account_id", QString::fromUtf8 (accountId));
		GetFullDraft_.bindValue (":draft_id", draftId);
		if (!GetFullDraft_.exec ())
		{
			Util::DBLock::DumpError (GetFullDraft_);
			throw std::runtime_error ("unable to get full draft");
		}

		GetFullDraft_.next ();
		Entry e;
		e.EntryId_ = draftId;
		e.Content_ = GetFullDraft_.value (1).toString ();
		e.Date_ = GetFullDraft_.value (2).toDateTime ();
		e.Subject_ = GetFullDraft_.value (3).toString ();
		GetFullDraft_.finish ();

		GetDraftTags_.bindValue (":draft_id", draftId);
		e.Tags_ = GetTags (GetDraftTags_);

		GetDraftPostOptions_.bindValue (":draft_id", draftId);
		e.PostOptions_ = GetEntryOptions (GetDraftPostOptions_);

		GetDraftCustomOptions_.bindValue (":draft_id", draftId);
		e.CustomData_ = GetEntryOptions (GetDraftCustomOptions_);

		return e;
	}

	QList<Entry> LocalStorage::GetShortDrafts (const QByteArray& accountId)
	{
		GetShortDrafts_.bindValue (":account_id", QString::fromUtf8 (accountId));
		if (!GetShortDrafts_.exec ())
		{
			Util::DBLock::DumpError (GetShortDrafts_);
			throw std::runtime_error ("unable to get short drafts");
		}

		QList<Entry> list;
		while (GetShortDrafts_.next ())
		{
			Entry e;
			e.EntryId_ = GetShortDrafts_.value (0).toLongLong ();
			e.Date_ = GetShortDrafts_.value (1).toDateTime ();
			e.Subject_ = GetShortDrafts_.value (2).toString ();

			list << e;
		}

		GetShortDrafts_.finish ();

		return list;
	}

	void LocalStorage::RemoveDraft (const QByteArray& accountId, qlonglong id)
	{
		RemoveDraft_.bindValue (":account_id", accountId);
		RemoveDraft_.bindValue (":id", id);
		if (!RemoveDraft_.exec ())
		{
			Util::DBLock::DumpError (RemoveDraft_);
			throw std::runtime_error ("unable to remove draft");
		}
	}

	void LocalStorage::SaveEntries (const QByteArray& accountID,
			const QList<Entry>& entries)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		for (const auto& entry : entries)
		{
			AddEntry_.bindValue (":account_id", QString::fromUtf8 (accountID));
			AddEntry_.bindValue (":entry_id", entry.EntryId_);
			AddEntry_.bindValue (":entry", entry.Content_);
			AddEntry_.bindValue (":date", entry.Date_);
			AddEntry_.bindValue (":subject", entry.Subject_);
			if (!AddEntry_.exec ())
			{
				Util::DBLock::DumpError (AddEntry_);
				throw std::runtime_error ("unable to add entry");
			}

			RemoveEntryTags_.bindValue (":entry_id", entry.EntryId_);
			if (!RemoveEntryTags_.exec ())
			{
				Util::DBLock::DumpError (RemoveEntryTags_);
				throw std::runtime_error ("unable to remove entry's tags");
			}

			for (const auto& tag : entry.Tags_)
			{
				if (tag.isEmpty ())
					continue;

				AddEntryTag_.bindValue (":tag", tag);
				AddEntryTag_.bindValue (":entry_id", entry.EntryId_);
				if (!AddEntryTag_.exec ())
				{
					Util::DBLock::DumpError (AddEntryTag_);
					throw std::runtime_error ("unable to add entry's tag");
				}
			}

			for (const auto& key : entry.PostOptions_.keys ())
			{
				AddEntryPostOptions_.bindValue (":entry_id", entry.EntryId_);
				AddEntryPostOptions_.bindValue (":name", key);
				AddEntryPostOptions_.bindValue (":value", entry.PostOptions_.value (key));
				if (!AddEntryPostOptions_.exec ())
				{
					Util::DBLock::DumpError (AddEntryPostOptions_);
					throw std::runtime_error ("unable to add entry's' params");
				}
			}

			for (const auto& key : entry.CustomData_.keys ())
			{
				AddEntryCustomOptions_.bindValue (":entry_id", entry.EntryId_);
				AddEntryCustomOptions_.bindValue (":name", key);
				AddEntryCustomOptions_.bindValue (":value", entry.CustomData_.value (key));
				if (!AddEntryCustomOptions_.exec ())
				{
					Util::DBLock::DumpError (AddEntryCustomOptions_);
					throw std::runtime_error ("unable to add entry's params");
				}
			}
		}

		lock.Good ();
	}

	void LocalStorage::UpdateEntry (const Entry& e)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		UpdateEntry_.bindValue (":entry_id", e.EntryId_);
		UpdateEntry_.bindValue (":entry", e.Content_);
		UpdateEntry_.bindValue (":date", e.Date_);
		UpdateEntry_.bindValue (":subject", e.Subject_);
		if (!UpdateEntry_.exec ())
		{
			Util::DBLock::DumpError (UpdateEntry_);
			throw std::runtime_error ("unable to update entry");
		}

		RemoveEntryTags_.bindValue (":entry_id", e.EntryId_);
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
			AddEntryTag_.bindValue (":entry_id", e.EntryId_);
			if (!AddEntryTag_.exec ())
			{
				Util::DBLock::DumpError (AddEntryTag_);
				throw std::runtime_error ("unable to add entry's tag");
			}
		}

		for (const auto& key : e.PostOptions_.keys ())
		{
			UpdateEntryPostOptions_.bindValue (":entry_id", e.EntryId_);
			UpdateEntryPostOptions_.bindValue (":name", key);
			UpdateEntryPostOptions_.bindValue (":value", e.PostOptions_.value (key));
			if (!UpdateEntryPostOptions_.exec ())
			{
				Util::DBLock::DumpError (UpdateEntryPostOptions_);
				throw std::runtime_error ("unable to update entry's options");
			}
		}

		for (const auto& key : e.CustomData_.keys ())
		{
			UpdateEntryCustomOptions_.bindValue (":entry_id", e.EntryId_);
			UpdateEntryCustomOptions_.bindValue (":name", key);
			UpdateEntryCustomOptions_.bindValue (":value", e.PostOptions_.value (key));
			if (!UpdateEntryCustomOptions_.exec ())
			{
				Util::DBLock::DumpError (UpdateEntryCustomOptions_);
				throw std::runtime_error ("unable to update entry's options");
			}
		}

		lock.Good ();
	}

	void LocalStorage::RemoveEntry (const QByteArray& accountId, qlonglong id)
	{
		Util::DBLock lock (DB_);
		lock.Init ();

		RemoveEntry_.bindValue (":entry_id", id);
		RemoveEntry_.bindValue (":account_id", QString::fromUtf8 (accountId));
		if (!RemoveEntry_.exec ())
		{
			Util::DBLock::DumpError (RemoveEntry_);
			throw std::runtime_error ("unable to remove entry");
		}

		lock.Good ();
	}

	Entry LocalStorage::GetEntry (const QByteArray& accountId, qlonglong entryId)
	{
		GetEntry_.bindValue (":account_id", QString::fromUtf8 (accountId));
		GetEntry_.bindValue (":entry_id", entryId);
		if (!GetEntry_.exec ())
		{
			Util::DBLock::DumpError (GetEntry_);
			throw std::runtime_error ("unable to get entry");
		}

		GetEntry_.next ();
		Entry e;
		e.EntryId_ = GetEntry_.value (1).toLongLong ();
		e.Content_ = GetEntry_.value (2).toString ();
		e.Date_ = GetEntry_.value (3).toDateTime ();
		e.Subject_ = GetEntry_.value (4).toString ();
		GetEntry_.finish ();

		GetEntryTags_.bindValue (":entry_id", entryId);
		e.Tags_ = GetTags (GetEntryTags_);

		GetEntryPostOptions_.bindValue (":entry_id", entryId);
		e.PostOptions_ = GetEntryOptions (GetEntryPostOptions_);

		GetEntryCustomOptions_.bindValue (":entry_id", entryId);
		e.CustomData_ = GetEntryOptions (GetEntryCustomOptions_);

		return e;
	}

	QList<Entry> LocalStorage::GetAllEntries (const QByteArray& accountId)
	{
		GetAllEntries_.bindValue (":account_id", QString::fromUtf8 (accountId));
		if (!GetAllEntries_.exec ())
		{
			Util::DBLock::DumpError (GetAllEntries_);
			throw std::runtime_error ("unable to get entries");
		}

		QList<Entry> list;
		while (GetAllEntries_.next ())
		{
			Entry e;
			e.EntryId_ = GetAllEntries_.value (1).toLongLong ();
			e.Content_ = GetAllEntries_.value (2).toString ();
			e.Date_ = GetAllEntries_.value (3).toDateTime ();
			e.Subject_ = GetAllEntries_.value (4).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);

			GetEntryPostOptions_.bindValue (":entry_id", e.EntryId_);
			e.PostOptions_ = GetEntryOptions (GetEntryPostOptions_);

			GetEntryCustomOptions_.bindValue (":entry_id", e.EntryId_);
			e.CustomData_ = GetEntryOptions (GetEntryCustomOptions_);

			list << e;
		}
		GetAllEntries_.finish ();

		return list;
	}

	QList<Entry> LocalStorage::GetLastEntries (const QByteArray& accountId, int count)
	{
		GetLastEntries_.bindValue (":account_id", QString::fromUtf8 (accountId));
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
			e.EntryId_ = GetLastEntries_.value (1).toLongLong ();
			e.Content_ = GetLastEntries_.value (2).toString ();
			e.Date_ = GetLastEntries_.value (3).toDateTime ();
			e.Subject_ = GetLastEntries_.value (4).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);

			GetEntryPostOptions_.bindValue (":entry_id", e.EntryId_);
			e.PostOptions_ = GetEntryOptions (GetEntryPostOptions_);

			GetEntryCustomOptions_.bindValue (":entry_id", e.EntryId_);
			e.CustomData_ = GetEntryOptions (GetEntryCustomOptions_);

			list << e;
		}
		GetLastEntries_.finish ();

		return list;
	}

	QList<Entry> LocalStorage::GetEntriesByDate (const QByteArray& accountId, const QDate& date)
	{
		GetEntriesByDate_.bindValue (":account_id", QString::fromUtf8 (accountId));
		GetEntriesByDate_.bindValue (":date", date.toString ("yyyy-MM-dd"));
		if (!GetEntriesByDate_.exec ())
		{
			Util::DBLock::DumpError (GetEntriesByDate_);
			throw std::runtime_error ("unable to get entries");
		}

		QList<Entry> list;
		while (GetEntriesByDate_.next ())
		{
			Entry e;
			e.EntryId_ = GetEntriesByDate_.value (1).toLongLong ();
			e.Content_ = GetEntriesByDate_.value (2).toString ();
			e.Date_ = GetEntriesByDate_.value (3).toDateTime ();
			e.Subject_ = GetEntriesByDate_.value (4).toString ();

			GetEntryTags_.bindValue (":entry_id", e.EntryId_);
			e.Tags_ = GetTags (GetEntryTags_);

			GetEntryPostOptions_.bindValue (":entry_id", e.EntryId_);
			e.PostOptions_ = GetEntryOptions (GetEntryPostOptions_);

			GetEntryCustomOptions_.bindValue (":entry_id", e.EntryId_);
			e.CustomData_ = GetEntryOptions (GetEntryCustomOptions_);

			list << e;
		}
		GetEntriesByDate_.finish ();

		return list;
	}

	QMap<QDate, int> LocalStorage::GetEntriesCountByDate (const QByteArray& accountId)
	{
		GetEntriesCountByDate_.bindValue (":account_id", QString::fromUtf8 (accountId));
		if (!GetEntriesCountByDate_.exec ())
		{
			Util::DBLock::DumpError (GetEntriesCountByDate_);
			throw std::runtime_error ("unable to get entries");
		}

		QMap<QDate, int> statistic;
		while (GetEntriesCountByDate_.next ())
			statistic.insert (GetEntriesCountByDate_.value (0).toDate (),
					GetEntriesCountByDate_.value (1).toInt ());
		GetAllEntries_.finish ();

		return statistic;
	}

	void LocalStorage::MoveFromEntriesToDrafts (const QByteArray& accId, int itemId)
	{
		const auto& e = GetEntry (accId, itemId);
		RemoveEntry (accId, e.EntryId_);
		SaveDraft (accId, e);
	}

	void LocalStorage::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["accounts"] = "CREATE TABLE IF NOT EXISTS accounts ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountID TEXT NOT NULL UNIQUE "
				");";
		table2query ["drafts"] = "CREATE TABLE IF NOT EXISTS drafts ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountID INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"Entry TEXT, "
				"Date DATE, "
				"Subject TEXT "
				");";
		table2query ["entries"] = "CREATE TABLE IF NOT EXISTS entries ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountID INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"EntryId INTEGER NOT NULL UNIQUE, "
				"Entry TEXT, "
				"Date DATE, "
				"Subject TEXT "
				");";
		table2query ["draft_post_options"] = "CREATE TABLE IF NOT EXISTS draft_post_options ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"DraftID INTEGER NOT NULL REFERENCES drafts (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
				");";
		table2query ["draft_custom_options"] = "CREATE TABLE IF NOT EXISTS draft_custom_options ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"DraftID INTEGER NOT NULL REFERENCES drafts (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
				");";
		table2query ["draft_tags"] = "CREATE TABLE IF NOT EXISTS draft_tags ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Tag TEXT NOT NULL, "
				"DraftId INTEGER NOT NULL REFERENCES drafts (Id) ON DELETE CASCADE "
				");";
		table2query ["entry_post_options"] = "CREATE TABLE IF NOT EXISTS entry_post_options ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"EntryID INTEGER NOT NULL REFERENCES entries (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
				");";
		table2query ["entry_custom_options"] = "CREATE TABLE IF NOT EXISTS entry_custom_options ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"EntryID INTEGER NOT NULL REFERENCES entries (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
				");";
		table2query ["entry_tags"] = "CREATE TABLE IF NOT EXISTS entry_tags ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Tag TEXT NOT NULL, "
				"EntryId INTEGER NOT NULL REFERENCES entries (Id) ON DELETE CASCADE "
				");";

		Util::DBLock lock (DB_);

		lock.Init ();

		const auto& tables = DB_.tables ();
		Q_FOREACH (const QString& key, table2query.keys ())
		if (!tables.contains (key))
		{
			QSqlQuery q (DB_);
			if (!q.exec (table2query [key]))
			{
				Util::DBLock::DumpError (q);
				throw std::runtime_error ("cannot create required tables");
			}
		}

		lock.Good ();
	}

	void LocalStorage::PrepareQueries ()
	{
		AddAccount_ = QSqlQuery (DB_);
		AddAccount_.prepare ("INSERT OR IGNORE INTO accounts (AccountID) "
				"VALUES (:account_id);");

		AddDraft_ = QSqlQuery (DB_);
		AddDraft_.prepare ("INSERT INTO drafts (AccountID, Entry, Date, Subject) "
				"VALUES ((SELECT accounts.ID FROM accounts WHERE accounts.AccountID = :account_id), "
				":entry, :date, :subject);");
		UpdateDraft_ = QSqlQuery (DB_);
		UpdateDraft_.prepare ("UPDATE drafts SET Entry = :entry, Date = :date, "
				"Subject = :subject WHERE Id = :draft_id;");
		RemoveDraft_ = QSqlQuery (DB_);
		RemoveDraft_.prepare ("DELETE FROM drafts WHERE Id = :id AND"
				" AccountID = (SELECT accounts.Id FROM accounts"
				" WHERE accounts.AccountID = :account_id);");
		GetDrafts_ = QSqlQuery (DB_);
		GetDrafts_.prepare ("SELECT Id, Entry, Date, Subject FROM drafts "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID = :account_id) ORDER BY Date DESC;");
		GetShortDrafts_ = QSqlQuery (DB_);
		GetShortDrafts_.prepare ("SELECT Id, Date, Subject FROM drafts "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID=:account_id) ORDER BY Date DESC;");
		GetFullDraft_ = QSqlQuery (DB_);
		GetFullDraft_.prepare ("SELECT Id, Entry, Date, Subject FROM drafts "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID = :account_id) AND Id = :draft_id;");

		AddDraftPostOptions_ = QSqlQuery (DB_);
		AddDraftPostOptions_.prepare ("INSERT INTO draft_post_options "
				"(DraftId, Name, Value) VALUES (:draft_id, :name, :value);");
		UpdateDraftPostOptions_ = QSqlQuery (DB_);
		UpdateDraftPostOptions_.prepare ("UPDATE draft_post_options SET Value = :value "
				"WHERE DraftId = :draft_id AND Name = :name;");
		GetDraftPostOptions_ = QSqlQuery (DB_);
		GetDraftPostOptions_.prepare ("SELECT Id, Name, Value FROM draft_post_options "
				"WHERE DraftID = :draft_id;");

		AddDraftCustomOptions_ = QSqlQuery (DB_);
		AddDraftCustomOptions_.prepare ("INSERT INTO draft_custom_options "
				"(DraftId, Name, Value) VALUES (:draft_id, :name, :value);");
		UpdateDraftCustomOptions_ = QSqlQuery (DB_);
		UpdateDraftCustomOptions_.prepare ("UPDATE draft_custom_options "
				"SET Value = :val WHERE DraftId = :draft_id AND Name = :name;");
		GetDraftCustomOptions_ = QSqlQuery (DB_);
		GetDraftCustomOptions_.prepare ("SELECT Id, Name, Value FROM draft_custom_options "
				"WHERE DraftID = :draft_id;");

		AddDraftTag_ = QSqlQuery (DB_);
		AddDraftTag_.prepare ("INSERT INTO draft_tags "
				"(Tag, DraftId) VALUES (:tag, :draft_id);");
		RemoveDraftTags_ = QSqlQuery (DB_);
		RemoveDraftTags_.prepare ("DELETE FROM draft_tags WHERE DraftID = :draft_id;");
		GetDraftTags_ = QSqlQuery (DB_);
		GetDraftTags_.prepare ("SELECT Id, Tag FROM draft_tags WHERE DraftID = :draft_id;");

		AddEntry_ = QSqlQuery (DB_);
		AddEntry_.prepare ("INSERT OR REPLACE INTO entries (AccountID, EntryId, Entry, Date, Subject) "
				"VALUES ((SELECT accounts.ID FROM accounts WHERE accounts.AccountID = :account_id), "
				":entry_id, :entry, :date, :subject);");
		UpdateEntry_ = QSqlQuery (DB_);
		UpdateEntry_.prepare ("UPDATE entries SET Entry = :entry, Date = :date, "
				"Subject = :subject WHERE EntryId = :entry_id;");
		RemoveEntry_ = QSqlQuery (DB_);
		RemoveEntry_.prepare ("DELETE FROM entries WHERE AccountID = ("
				"SELECT accounts.ID FROM accounts WHERE accounts.AccountID = :account_id)"
				" AND EntryId = :entry_id;");
		GetEntry_ = QSqlQuery (DB_);
		GetEntry_.prepare ("SELECT Id, EntryId, Entry, Date, Subject FROM entries "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID = :account_id) AND EntryId = :entry_id");
		GetAllEntries_ = QSqlQuery (DB_);
		GetAllEntries_.prepare ("SELECT Id, EntryId,  Entry, Date, Subject FROM entries "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID = :account_id) ORDER BY Date DESC;");
		GetLastEntries_= QSqlQuery (DB_);
		GetLastEntries_.prepare ("SELECT Id, EntryId, Entry, Date, Subject FROM entries "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID = :account_id) ORDER BY Date DESC LIMIT :limit;");
		GetEntriesByDate_= QSqlQuery (DB_);;
		GetEntriesByDate_.prepare ("SELECT Id, EntryId, Entry, Date, Subject FROM entries "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID = :account_id) AND date (Date) = :date;");
		GetEntriesCountByDate_ = QSqlQuery (DB_);
		GetEntriesCountByDate_.prepare ("SELECT date (Date), COUNT (Id) FROM entries "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID = :account_id) GROUP BY date (Date);");

		AddEntryPostOptions_ = QSqlQuery (DB_);
		AddEntryPostOptions_.prepare ("INSERT INTO entry_post_options "
				"(EntryId, Name, Value) VALUES ((SELECT Id FROM entries "
				"WHERE EntryId = :entry_id), :name, :value);");
		UpdateEntryPostOptions_ = QSqlQuery (DB_);
		UpdateEntryPostOptions_.prepare ("UPDATE entry_post_options SET Value = :value"
				" WHERE EntryId = (SELECT Id FROM entries WHERE EntryId = :entry_id)"
				" AND Name = :name;");
		GetEntryPostOptions_ = QSqlQuery (DB_);
		GetEntryPostOptions_.prepare ("SELECT Id, Name, Value FROM entry_post_options "
				"WHERE EntryId = (SELECT Id FROM entries WHERE EntryId = :entry_id);");

		AddEntryCustomOptions_ = QSqlQuery (DB_);
		AddEntryCustomOptions_.prepare ("INSERT INTO entry_custom_options "
				"(EntryId, Name, Value) VALUES (SELECT Id FROM entries "
				"WHERE EntryId = :entry_id, :name, :value);");
		UpdateEntryCustomOptions_ = QSqlQuery (DB_);
		UpdateEntryCustomOptions_.prepare ("UPDATE entry_custom_options"
				" SET Value = :val  WHERE EntryId = (SELECT Id FROM entries"
				" WHERE EntryId = :entry_id) AND Name = :name;");
		GetEntryCustomOptions_ = QSqlQuery (DB_);
		GetEntryCustomOptions_.prepare ("SELECT Id, Name, Value FROM entry_custom_options "
				"WHERE EntryId = (SELECT Id FROM entries WHERE EntryId = :entry_id);");

		AddEntryTag_ = QSqlQuery (DB_);
		AddEntryTag_.prepare ("INSERT INTO entry_tags "
				"(Tag, EntryID) VALUES (:tag, (SELECT Id FROM entries "
				"WHERE EntryId = :entry_id));");
		RemoveEntryTags_ = QSqlQuery (DB_);
		RemoveEntryTags_.prepare ("DELETE FROM entry_tags WHERE EntryID = ("
				" SELECT Id FROM entries WHERE EntryId = :entry_id);");
		GetEntryTags_ = QSqlQuery (DB_);
		GetEntryTags_.prepare ("SELECT Id, Tag FROM entry_tags WHERE EntryID = ("
				" SELECT Id FROM entries WHERE EntryId = :entry_id);");
	}
}
}
