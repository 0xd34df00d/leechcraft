/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "storagemanager.h"
#include <stdexcept>
#include <QtDebug>
#include <QSqlError>
#include <util/util.h>
#include <util/dblock.h>

namespace LeechCraft
{
namespace Blogique
{
	StorageManager::StorageManager (QObject *parent)
	: QObject (parent)
	, BlogiqueDB_ (QSqlDatabase::addDatabase ("QSQLITE",
		QString ("Blogique_DataBase")))
	{
		BlogiqueDB_.setDatabaseName (Util::CreateIfNotExists ("blogique")
				.filePath ("blogique.db"));

		if (!BlogiqueDB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (BlogiqueDB_.lastError ());
			throw std::runtime_error ("unable to open Blogique database");
		}

		{
			QSqlQuery query (BlogiqueDB_);
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

	void StorageManager::AddAccount (const QByteArray& accounId)
	{
		Util::DBLock lock (BlogiqueDB_);
		lock.Init ();

		AddAccount_.bindValue (":account_id", QString::fromUtf8 (accounId));
		if (!AddAccount_.exec ())
		{
			Util::DBLock::DumpError (AddAccount_);
			throw std::runtime_error ("unable to add account");
		}

		lock.Good ();
	}

	qint64 StorageManager::SaveNewDraft (const Entry& e)
	{
		Util::DBLock lock (BlogiqueDB_);
		lock.Init ();

		AddDraft_.bindValue (":entry", e.Content_);
		AddDraft_.bindValue (":date", e.Date_);
		AddDraft_.bindValue (":subject", e.Subject_);
		if (!AddDraft_.exec ())
		{
			Util::DBLock::DumpError (AddDraft_);
			throw std::runtime_error ("unable to add draft");
		}

		const qint64 id = AddDraft_.lastInsertId ().toLongLong ();

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

	void StorageManager::RemoveDraft (qint64 draftId)
	{
		RemoveDraft_.bindValue (":draft_id", draftId);
		if (!RemoveDraft_.exec ())
		{
			Util::DBLock::DumpError (RemoveDraft_);
			throw std::runtime_error ("unable to remove draft");
		}
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

	QList<Entry> StorageManager::GetDrafts (Mode mode)
	{
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

			if (mode == Mode::FullMode)
			{
				GetDraftTags_.bindValue (":draft_id", draftId);
				e.Tags_ = GetTags (GetDraftTags_);

				GetDraftPostOptions_.bindValue (":draft_id", draftId);
				e.PostOptions_ = GetEntryOptions (GetDraftPostOptions_);

				GetDraftCustomOptions_.bindValue (":draft_id", draftId);
				e.CustomData_ = GetEntryOptions (GetDraftCustomOptions_);
			}
			list << e;
		}
		GetDrafts_.finish ();

		return list;
	}

	QMap<QDate, int> StorageManager::GetDraftsCountByDate ()
	{

	}

	Entry StorageManager::GetFullDraft (qint64 draftId)
	{

	}

	void StorageManager::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["accounts"] = "CREATE TABLE IF NOT EXISTS accounts ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountID TEXT NOT NULL UNIQUE "
				");";
		table2query ["drafts"] = "CREATE TABLE IF NOT EXISTS drafts ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
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

		Util::DBLock lock (BlogiqueDB_);

		lock.Init ();

		const auto& tables = BlogiqueDB_.tables ();
		Q_FOREACH (const QString& key, table2query.keys ())
		if (!tables.contains (key))
		{
			QSqlQuery q (BlogiqueDB_);
			if (!q.exec (table2query [key]))
			{
				Util::DBLock::DumpError (q);
				throw std::runtime_error ("cannot create required tables");
			}
		}

		lock.Good ();
	}

	void StorageManager::PrepareQueries ()
	{
		AddAccount_ = QSqlQuery (BlogiqueDB_);
		AddAccount_.prepare ("INSERT OR IGNORE INTO accounts (AccountID) "
				"VALUES (:account_id);");

		AddDraft_ = QSqlQuery (BlogiqueDB_);
		AddDraft_.prepare ("INSERT INTO drafts (Entry, Date, Subject) "
				"VALUES (:entry, :date, :subject);");
		UpdateDraft_ = QSqlQuery (BlogiqueDB_);
		UpdateDraft_.prepare ("UPDATE drafts SET Entry = :entry, Date = :date, "
				"Subject = :subject WHERE Id = :draft_id;");
		RemoveDraft_ = QSqlQuery (BlogiqueDB_);
		RemoveDraft_.prepare ("DELETE FROM drafts WHERE Id = :draft_id;");

		GetDrafts_ = QSqlQuery (BlogiqueDB_);
		GetDrafts_.prepare ("SELECT Id, Entry, Date, Subject FROM drafts "
				"ORDER BY Date DESC;");
		GetShortDrafts_ = QSqlQuery (BlogiqueDB_);
		GetShortDrafts_.prepare ("SELECT Id, Date, Subject FROM drafts "
				"ORDER BY Date DESC;");
		GetFullDraft_ = QSqlQuery (BlogiqueDB_);
		GetFullDraft_.prepare ("SELECT Id, Entry, Date, Subject FROM drafts "
				"WHERE Id = :draft_id;");

		AddDraftPostOptions_ = QSqlQuery (BlogiqueDB_);
		AddDraftPostOptions_.prepare ("INSERT INTO draft_post_options "
				"(DraftId, Name, Value) VALUES (:draft_id, :name, :value);");
		UpdateDraftPostOptions_ = QSqlQuery (BlogiqueDB_);
		UpdateDraftPostOptions_.prepare ("UPDATE draft_post_options SET Value = :value "
				"WHERE DraftId = :draft_id AND Name = :name;");
		GetDraftPostOptions_ = QSqlQuery (BlogiqueDB_);
		GetDraftPostOptions_.prepare ("SELECT Id, Name, Value FROM draft_post_options "
				"WHERE DraftID = :draft_id;");

		AddDraftCustomOptions_ = QSqlQuery (BlogiqueDB_);
		AddDraftCustomOptions_.prepare ("INSERT INTO draft_custom_options "
				"(DraftId, Name, Value) VALUES (:draft_id, :name, :value);");
		UpdateDraftCustomOptions_ = QSqlQuery (BlogiqueDB_);
		UpdateDraftCustomOptions_.prepare ("UPDATE draft_custom_options "
				"SET Value = :val WHERE DraftId = :draft_id AND Name = :name;");
		GetDraftCustomOptions_ = QSqlQuery (BlogiqueDB_);
		GetDraftCustomOptions_.prepare ("SELECT Id, Name, Value FROM draft_custom_options "
				"WHERE DraftID = :draft_id;");

		AddDraftTag_ = QSqlQuery (BlogiqueDB_);
		AddDraftTag_.prepare ("INSERT INTO draft_tags "
				"(Tag, DraftId) VALUES (:tag, :draft_id);");
		RemoveDraftTags_ = QSqlQuery (BlogiqueDB_);
		RemoveDraftTags_.prepare ("DELETE FROM draft_tags WHERE DraftID = :draft_id;");
		GetDraftTags_ = QSqlQuery (BlogiqueDB_);
		GetDraftTags_.prepare ("SELECT Id, Tag FROM draft_tags WHERE DraftID = :draft_id;");
	}

	void StorageManager::updateDraft (const Entry& e, qint64 draftId)
	{
		Util::DBLock lock (BlogiqueDB_);
		lock.Init ();

		UpdateDraft_.bindValue (":entry", e.Content_);
		UpdateDraft_.bindValue (":date", e.Date_);
		UpdateDraft_.bindValue (":subject", e.Subject_);
		UpdateDraft_.bindValue (":draft_id", draftId);
		if (!UpdateDraft_.exec ())
		{
			Util::DBLock::DumpError (UpdateDraft_);
			throw std::runtime_error ("unable to update draft");
		}

		RemoveDraftTags_.bindValue (":draft_id", draftId);
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
			AddDraftTag_.bindValue (":draft_id", draftId);
			if (!AddDraftTag_.exec ())
			{
				Util::DBLock::DumpError (AddDraftTag_);
				throw std::runtime_error ("unable to add draft's tag");
			}
		}

		for (const auto& key : e.PostOptions_.keys ())
		{
			UpdateDraftPostOptions_.bindValue (":draft_id", draftId);
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
			UpdateDraftCustomOptions_.bindValue (":draft_id", draftId);
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

}
}