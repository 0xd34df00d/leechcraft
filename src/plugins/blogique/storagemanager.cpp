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

	void StorageManager::RemoveAccount (const QByteArray& accounId)
	{
		Util::DBLock lock (BlogiqueDB_);
		lock.Init ();

		RemoveAccount_.bindValue (":account_id", QString::fromUtf8 (accounId));
		if (!RemoveAccount_.exec ())
		{
			Util::DBLock::DumpError (RemoveAccount_);
			throw std::runtime_error ("unable to remove account");
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

		lock.Good ();
		return id;
	}

	qint64 StorageManager::UpdateDraft (const Entry& e, qint64 draftId)
	{
		Util::DBLock lock (BlogiqueDB_);
		lock.Init ();

		Entry entry = GetFullDraft (draftId);
		if (entry.IsEmpty ())
			draftId = SaveNewDraft (e);
		else
		{
			UpdateDraft_.bindValue (":entry", e.Content_);
			UpdateDraft_.bindValue (":date", e.Date_);
			UpdateDraft_.bindValue (":subject", e.Subject_);
			UpdateDraft_.bindValue (":draft_id", draftId);
			if (!UpdateDraft_.exec ())
			{
				Util::DBLock::DumpError (UpdateDraft_);
				throw std::runtime_error ("unable to update draft");
			}
		}

		lock.Good ();

		return draftId;
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
			e.EntryId_ = GetDrafts_.value (0).toInt ();
			e.Content_ = GetDrafts_.value (1).toString ();
			e.Date_ = GetDrafts_.value (2).toDateTime ();
			e.Subject_ = GetDrafts_.value (3).toString ();

			list << e;
		}
		GetDrafts_.finish ();

		return list;
	}

	QList<Entry> StorageManager::GetDraftsByDate (const QDate& date)
	{
		GetDraftsByDate_.bindValue (":date", date.toString ("yyyy-MM-dd"));
		if (!GetDraftsByDate_.exec ())
		{
			Util::DBLock::DumpError (GetDraftsByDate_);
			throw std::runtime_error ("unable to get drafts");
		}

		QList<Entry> list;
		while (GetDraftsByDate_.next ())
		{
			Entry e;
			e.EntryId_ = GetDraftsByDate_.value (0).toInt ();
			e.Content_ = GetDraftsByDate_.value (1).toString ();
			e.Date_ = GetDraftsByDate_.value (2).toDateTime ();
			e.Subject_ = GetDraftsByDate_.value (3).toString ();

			list << e;
		}
		GetDraftsByDate_.finish ();

		return list;
	}

	QMap<QDate, int> StorageManager::GetDraftsCountByDate ()
	{
		if (!GetDraftsCountByDate_.exec ())
		{
			Util::DBLock::DumpError (GetDraftsCountByDate_);
			throw std::runtime_error ("unable to get entries");
		}

		QMap<QDate, int> statistic;
		while (GetDraftsCountByDate_.next ())
			statistic.insert (GetDraftsCountByDate_.value (0).toDate (),
					GetDraftsCountByDate_.value (1).toInt ());
		GetDraftsCountByDate_.finish ();

		return statistic;
	}

	Entry StorageManager::GetFullDraft (qint64 draftId)
	{
		GetFullDraft_.bindValue (":draft_id", draftId);
		if (!GetFullDraft_.exec ())
		{
			Util::DBLock::DumpError (GetFullDraft_);
			throw std::runtime_error ("unable to get full draft by id");
		}

		Entry e;
		while (GetFullDraft_.next ())
		{
			e.EntryId_ = draftId;
			e.Content_ = GetFullDraft_.value (1).toString ();
			e.Date_ = GetFullDraft_.value (2).toDateTime ();
			e.Subject_ = GetFullDraft_.value (3).toString ();
		}
		GetFullDraft_.finish ();

		return e;
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
		RemoveAccount_ = QSqlQuery (BlogiqueDB_);
		RemoveAccount_.prepare ("DELETE FROM accounts WHERE AccountID = :account_id;");

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
		GetDraftsByDate_ = QSqlQuery (BlogiqueDB_);
		GetDraftsByDate_.prepare ("SELECT Id, Entry, Date, Subject FROM drafts "
				"WHERE date (Date) = :date;");
		GetDraftsCountByDate_ = QSqlQuery (BlogiqueDB_);
		GetDraftsCountByDate_.prepare ("SELECT date (Date), COUNT (Id) FROM drafts "
				" GROUP BY date (Date);");
	}

}
}