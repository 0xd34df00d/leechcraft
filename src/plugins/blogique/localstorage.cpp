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
#include <boost/graph/graph_concepts.hpp>
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

	void LocalStorage::AddAccount (const QByteArray& accounId)
	{
		AddAccount_.bindValue (":account_id", QString (accounId));
		if (!AddAccount_.exec ())
		{
			Util::DBLock::DumpError (AddAccount_);
			throw std::runtime_error ("unable to add account");
		}
	}

	qlonglong LocalStorage::SaveDraft (const QByteArray& accountID, const Event& e)
	{
		AddDraft_.bindValue (":account_id", QString (accountID));
		AddDraft_.bindValue (":entry", e.Content_);
		AddDraft_.bindValue (":date", e.Date_);
		AddDraft_.bindValue (":subject", e.Subject_);
		AddDraft_.bindValue (":tags", e.Tags_);

		if (!AddDraft_.exec ())
		{
			Util::DBLock::DumpError (AddDraft_);
			throw std::runtime_error ("unable to add draft");
		}

		const int id = AddDraft_.lastInsertId ().toInt ();
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

		return id;
	}

	void LocalStorage::UpdateDraft (qlonglong id, const Event& e)
	{
		UpdateDraft_.bindValue (":entry", e.Content_);
		UpdateDraft_.bindValue (":date", e.Date_);
		UpdateDraft_.bindValue (":subject", e.Subject_);
		UpdateDraft_.bindValue (":tags", e.Tags_);
		UpdateDraft_.bindValue (":draft_id", id);
		if (!UpdateDraft_.exec ())
		{
			Util::DBLock::DumpError (UpdateDraft_);
			throw std::runtime_error ("unable to update draft");
		}

		Util::DBLock lock (DB_);
		lock.Init ();

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

	QList<Event> LocalStorage::GetDrafts (const QByteArray& accountId)
	{
		GetDrafts_.bindValue (":account_id", QString (accountId));
		if (!GetDrafts_.exec ())
		{
			Util::DBLock::DumpError (GetDrafts_);
			throw std::runtime_error ("unable to get drafts");
		}

		QList<Event> list;
		while (GetDrafts_.next ())
		{
			Event e;
			const int draftId = GetDrafts_.value (0).toInt ();
			e.EntryDBId_ = draftId;
			e.Content_ = GetDrafts_.value (1).toString ();
			e.Date_ = GetDrafts_.value (2).toDateTime ();
			e.Subject_ = GetDrafts_.value (3).toString ();
			e.Tags_ = GetDrafts_.value (4).toStringList ();

			GetDraftPostOptions_.bindValue (":draft_id", draftId);
			if (!GetDraftPostOptions_.exec ())
			{
				Util::DBLock::DumpError (GetDraftPostOptions_);
				throw std::runtime_error ("unable to get draft params");
			}

			while (GetDraftPostOptions_.next ())
				e.PostOptions_.insert (GetDraftPostOptions_.value (1).toString (),
						GetDraftPostOptions_.value (2));

			GetDraftCustomOptions_.bindValue (":draft_id", draftId);
			if (!GetDraftCustomOptions_.exec ())
			{
				Util::DBLock::DumpError (GetDraftCustomOptions_);
				throw std::runtime_error ("unable to get draft params");
			}

			while (GetDraftCustomOptions_.next ())
				e.CustomData_.insert (GetDraftCustomOptions_.value (1).toString (),
						GetDraftCustomOptions_.value (2));

			list << e;
		}

		return list;
	}

	Event LocalStorage::GetFullDraft (const QByteArray& accountId, qlonglong draftId)
	{
		GetFullDraft_.bindValue (":account_id", QString (accountId));
		GetFullDraft_.bindValue (":draft_id", draftId);
		if (!GetFullDraft_.exec ())
		{
			Util::DBLock::DumpError (GetFullDraft_);
			throw std::runtime_error ("unable to get full draft");
		}

		GetFullDraft_.next ();
		Event e;
		e.EntryDBId_ = draftId;
		e.Content_ = GetFullDraft_.value (1).toString ();
		e.Date_ = GetFullDraft_.value (2).toDateTime ();
		e.Subject_ = GetFullDraft_.value (3).toString ();
		e.Tags_ = GetFullDraft_.value (4).toStringList ();

		GetDraftPostOptions_.bindValue (":draft_id", draftId);
		if (!GetDraftPostOptions_.exec ())
		{
			Util::DBLock::DumpError (GetDraftPostOptions_);
			throw std::runtime_error ("unable to get draft params");
		}

		while (GetDraftPostOptions_.next ())
			e.PostOptions_.insert (GetDraftPostOptions_.value (1).toString (),
					GetDraftPostOptions_.value (2));

			GetDraftCustomOptions_.bindValue (":draft_id", draftId);
		if (!GetDraftCustomOptions_.exec ())
		{
			Util::DBLock::DumpError (GetDraftCustomOptions_);
			throw std::runtime_error ("unable to get draft params");
		}

		while (GetDraftCustomOptions_.next ())
			e.CustomData_.insert (GetDraftCustomOptions_.value (1).toString (),
					GetDraftCustomOptions_.value (2));

		return e;
	}

	QList<Event> LocalStorage::GetShortDrafts (const QByteArray& accountId)
	{
		GetShortDrafts_.bindValue (":account_id", QString (accountId));
		if (!GetShortDrafts_.exec ())
		{
			Util::DBLock::DumpError (GetShortDrafts_);
			throw std::runtime_error ("unable to get short drafts");
		}
		
		QList<Event> list;
		while (GetShortDrafts_.next ())
		{
			Event e;
			e.EntryDBId_ = GetShortDrafts_.value (0).toLongLong ();
			e.Date_ = GetShortDrafts_.value (1).toDateTime ();
			e.Subject_ = GetShortDrafts_.value (2).toString ();

			list << e;
		}

		return list;
	}

	void LocalStorage::RemoveDraft (qlonglong id)
	{
		RemoveDraft_.bindValue (":id", id);
		if (!RemoveDraft_.exec ())
		{
			Util::DBLock::DumpError (RemoveDraft_);
			throw std::runtime_error ("unable to remove draft");
		}
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
				"Subject TEXT, "
				"Tags TEXT "
				");";
		table2query ["entries"] = "CREATE TABLE IF NOT EXISTS entries ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountID INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"Entry TEXT, "
				"Date DATE, "
				"Subject TEXT, "
				"Tags TEXT "
				");";
		table2query ["draft_post_options"] = "CREATE TABLE IF NOT EXISTS "
				"draft_post_options (Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"DraftID INTEGER NOT NULL REFERENCES drafts (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
				");";
		table2query ["draft_custom_options"] = "CREATE TABLE IF NOT EXISTS "
				"draft_custom_options (Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"DraftID INTEGER NOT NULL REFERENCES drafts (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
				");";
		table2query ["entry_post_options"] = "CREATE TABLE IF NOT EXISTS "
				"entry_post_options (Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"EntryID INTEGER NOT NULL REFERENCES entries (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
				");";
		table2query ["entry_custom_options"] = "CREATE TABLE IF NOT EXISTS "
				"entry_custom_options (Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"EntryID INTEGER NOT NULL REFERENCES entries (Id) ON DELETE CASCADE, "
				"Name TEXT, "
				"Value TEXT "
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
		AddDraft_.prepare ("INSERT INTO drafts (AccountID, Entry, Date, Subject, Tags) "
				"VALUES ((SELECT accounts.ID FROM accounts WHERE accounts.AccountID=:account_id), "
				":entry, :date, :subject, :tags);");
		UpdateDraft_ = QSqlQuery (DB_);
		UpdateDraft_.prepare ("UPDATE drafts SET Entry=:entry, Date=:date, "
				"Subject=:subject, Tags=:tags WHERE Id=:draft_id;");

		AddDraftPostOptions_ = QSqlQuery (DB_);
		AddDraftPostOptions_.prepare ("INSERT INTO draft_post_options "
				"(DraftId, Name, Value) VALUES (:draft_id, :name, :value);");

		UpdateDraftPostOptions_ = QSqlQuery (DB_);
		UpdateDraftPostOptions_.prepare ("UPDATE draft_post_options SET Value=:value "
				"WHERE DraftId=:draft_id AND Name=:name;");
		
		AddDraftCustomOptions_ = QSqlQuery (DB_);
		AddDraftCustomOptions_.prepare ("INSERT INTO draft_custom_options "
				"(DraftId, Name, Value) VALUES (:draft_id, :name, :value);");

		UpdateDraftCustomOptions_ = QSqlQuery (DB_);
		UpdateDraftCustomOptions_.prepare ("UPDATE draft_custom_options "
				"SET Value=:val WHERE DraftId=:draft_id AND Name=:name;");

		GetShortDrafts_ = QSqlQuery (DB_);
		GetShortDrafts_.prepare ("SELECT Id, Date, Subject FROM drafts "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID=:account_id) ORDER BY Date;");

		GetDrafts_ = QSqlQuery (DB_);
		GetDrafts_.prepare ("SELECT Id, Entry, Date, Subject, Tags FROM drafts "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID=:account_id) ORDER BY Date;");

		GetFullDraft_ = QSqlQuery (DB_);
		GetFullDraft_.prepare ("SELECT Id, Entry, Date, Subject, Tags FROM drafts "
				"WHERE AccountID = (SELECT accounts.Id FROM accounts "
				"WHERE accounts.AccountID=:account_id) AND Id=:draft_id;");

		GetDraftPostOptions_ = QSqlQuery (DB_);
		GetDraftPostOptions_.prepare ("SELECT Id, Name, Value FROM draft_post_options "
				"WHERE DraftID = :draft_id;");

		GetDraftCustomOptions_ = QSqlQuery (DB_);
		GetDraftCustomOptions_.prepare ("SELECT Id, Name, Value FROM draft_custom_options "
				"WHERE DraftID = :draft_id;");

		RemoveDraft_ = QSqlQuery (DB_);
		RemoveDraft_.prepare ("DELETE FROM drafts WHERE Id=:id;");
	}
}
}