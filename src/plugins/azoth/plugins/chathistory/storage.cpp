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

#include "storage.h"
#include <stdexcept>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <util/util.h>
#include <util/dblock.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	Storage::RawSearchResult::RawSearchResult ()
	: EntryID_ (0)
	, AccountID_ (0)
	{
	}

	Storage::RawSearchResult::RawSearchResult (qint32 entryId, qint32 accountId, const QDateTime& date)
	: EntryID_ (entryId)
	, AccountID_ (accountId)
	, Date_ (date)
	{
	}

	bool Storage::RawSearchResult::IsEmpty () const
	{
		return Date_.isNull () || !EntryID_ || !AccountID_;
	}

	Storage::Storage (QObject *parent)
	: QObject (parent)
	{
		DB_.reset (new QSqlDatabase (QSqlDatabase::addDatabase ("QSQLITE", "History connection")));
		DB_->setDatabaseName (Util::CreateIfNotExists ("azoth").filePath ("history.db"));
		if (!DB_->open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (DB_->lastError ());
			throw std::runtime_error ("unable to open Azoth history database");
		}

		InitializeTables ();

		UserSelector_ = QSqlQuery (*DB_);
		UserSelector_.prepare ("SELECT Id, EntryID FROM azoth_users");

		AccountSelector_ = QSqlQuery (*DB_);
		AccountSelector_.prepare ("SELECT Id, AccountID FROM azoth_accounts");

		UserIDSelector_ = QSqlQuery (*DB_);
		UserIDSelector_.prepare ("SELECT Id FROM azoth_users WHERE EntryID = :entry_id");

		AccountIDSelector_ = QSqlQuery (*DB_);
		AccountIDSelector_.prepare ("SELECT Id FROM azoth_accounts WHERE AccountID = :account_id");

		UserInserter_ = QSqlQuery (*DB_);
		UserInserter_.prepare ("INSERT INTO azoth_users (EntryID) VALUES (:entry_id);");

		AccountInserter_ = QSqlQuery (*DB_);
		AccountInserter_.prepare ("INSERT INTO azoth_accounts (AccountID) VALUES (:account_id);");

		MessageDumper_ = QSqlQuery (*DB_);
		MessageDumper_.prepare ("INSERT INTO azoth_history (Id, AccountID, Date, Direction, Message, Variant, Type) "
				"VALUES (:id, :account_id, :date, :direction, :message, :variant, :type);");

		UsersForAccountGetter_ = QSqlQuery (*DB_);
		UsersForAccountGetter_.prepare ("SELECT DISTINCT azoth_history.Id, EntryID FROM azoth_users, azoth_history "
				"WHERE azoth_history.Id = azoth_users.Id AND AccountID = :account_id;");

		Date2Pos_ = QSqlQuery (*DB_);
		Date2Pos_.prepare ("SELECT COUNT(1) FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"AND Date >= :date");

		LogsSearcher_ = QSqlQuery (*DB_);
		LogsSearcher_.prepare ("SELECT date FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"AND Date = (SELECT Date FROM azoth_history "
				"	WHERE Id = :inner_entry_id "
				"	AND AccountID = :inner_account_id "
				"	AND Message LIKE :text "
				"	ORDER BY Date DESC "
				"	LIMIT 1 OFFSET :offset);");

		LogsSearcherWOContact_ = QSqlQuery (*DB_);
		LogsSearcherWOContact_.prepare ("SELECT Date, Id FROM azoth_history "
				"WHERE AccountID = :account_id "
				"AND Date = (SELECT Date FROM azoth_history "
				"	WHERE AccountID = :inner_account_id "
				"	AND Message LIKE :text "
				"	ORDER BY Date DESC "
				"	LIMIT 1 OFFSET :offset);");

		LogsSearcherWOContactAccount_ = QSqlQuery (*DB_);
		LogsSearcherWOContactAccount_.prepare ("SELECT Date, Id, AccountID FROM azoth_history "
				"WHERE Date = (SELECT Date FROM azoth_history "
				"	WHERE Message LIKE :text "
				"	ORDER BY Date DESC "
				"	LIMIT 1 OFFSET :offset);");

		HistoryGetter_ = QSqlQuery (*DB_);
		HistoryGetter_.prepare ("SELECT Date, Direction, Message, Variant, Type "
				"FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"ORDER BY Date DESC LIMIT :limit OFFSET :offset;");

		HistoryClearer_ = QSqlQuery (*DB_);
		HistoryClearer_.prepare ("DELETE FROM azoth_history WHERE Id = :entry_id AND AccountID = :account_id;");

		EntryCacheGetter_ = QSqlQuery (*DB_);
		EntryCacheGetter_.prepare ("SELECT VisibleName FROM azoth_entrycache WHERE Id = :id;");

		EntryCacheSetter_ = QSqlQuery (*DB_);
		EntryCacheSetter_.prepare ("INSERT OR REPLACE INTO azoth_entrycache (Id, VisibleName) "
				"VALUES (:id, :visible_name);");

		try
		{
			Users_ = GetUsers ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get saved users, we would be a bit more inefficient";
		}

		try
		{
			Accounts_ = GetAccounts ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get saved accounts, we would be a bit more inefficient";
		}
	}

	void Storage::InitializeTables ()
	{
		Util::DBLock lock (*DB_);
		try
		{
			lock.Init ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error locking database for transaction:"
					<< e.what ();
			throw;
		}

		QSqlQuery query (*DB_);
		QMap<QString, QString> table2query;
		table2query ["azoth_users"] = "CREATE TABLE azoth_users ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"EntryID TEXT "
					");";
		table2query ["azoth_accounts"] = "CREATE TABLE azoth_accounts ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"AccountID TEXT "
					");";
		table2query ["azoth_history"] = "CREATE TABLE azoth_history ("
					"Id INTEGER, "
					"AccountId INTEGER, "
					"Date DATETIME, "
					"Direction INTEGER, "
					"Message TEXT, "
					"Variant TEXT, "
					"Type INTEGER, "
					"UNIQUE (Id, AccountId, Date, Direction, Message, Variant, Type) ON CONFLICT IGNORE);";
		table2query ["azoth_entrycache"] = "CREATE TABLE azoth_entrycache ("
					"Id INTEGER UNIQUE ON CONFLICT REPLACE REFERENCES azoth_users (Id), "
					"VisibleName TEXT "
					");";
		const QStringList& tables = DB_->tables ();
		Q_FOREACH (const QString& table, table2query.keys ())
		{
			if (tables.contains (table))
				continue;

			const QString& queryStr = table2query [table];
			if (!query.exec (queryStr))
			{
				Util::DBLock::DumpError (query);
				throw std::runtime_error ("Unable to create tables for Azoth history");
			}
		}

		lock.Good ();
	}

	QHash<QString, qint32> Storage::GetUsers ()
	{
		if (!UserSelector_.exec ())
		{
			Util::DBLock::DumpError (UserSelector_);
			throw std::runtime_error ("Unable to perform user selection for Azoth history");
		}

		QHash<QString, qint32> result;
		while (UserSelector_.next ())
			result [UserSelector_.value (1).toString ()] =
					UserSelector_.value (0).toInt ();

		return result;
	}

	qint32 Storage::GetUserID (const QString& entryId)
	{
		UserIDSelector_.bindValue (":entry_id", entryId);
		if (!UserIDSelector_.exec ())
		{
			Util::DBLock::DumpError (UserIDSelector_);
			throw std::runtime_error ("ChatHistory::Storage::GetUserID: unable to get user's ID");
		}

		if (!UserIDSelector_.next ())
			return -1;

		qint32 result = UserIDSelector_.value (0).toInt ();
		UserIDSelector_.finish ();
		return result;
	}

	void Storage::AddUser (const QString& id)
	{
		UserInserter_.bindValue (":entry_id", id);
		if (!UserInserter_.exec ())
		{
			Util::DBLock::DumpError (UserInserter_);
			return;
		}
		UserInserter_.finish ();

		try
		{
			Users_ [id] = GetUserID (id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to refetch id for"
					<< id
					<< "with:"
					<< e.what ();
		}
	}

	QHash<QString, qint32> Storage::GetAccounts()
	{
		if (!AccountSelector_.exec ())
		{
			Util::DBLock::DumpError (AccountSelector_);
			throw std::runtime_error ("Unable to perform account selection for Azoth history");
		}

		QHash<QString, qint32> result;
		while (AccountSelector_.next ())
			result [AccountSelector_.value (1).toString ()] =
					AccountSelector_.value (0).toInt ();

		return result;
	}

	qint32 Storage::GetAccountID (const QString& accId)
	{
		AccountIDSelector_.bindValue (":account_id", accId);
		if (!AccountIDSelector_.exec ())
		{
			Util::DBLock::DumpError (AccountIDSelector_);
			throw std::runtime_error ("ChatHistory::Storage::GetAccountID: unable to get account ID");
		}

		if (!AccountIDSelector_.next ())
			return -1;

		qint32 result = AccountIDSelector_.value (0).toInt ();
		AccountIDSelector_.finish ();
		return result;
	}

	void Storage::AddAccount (const QString& id)
	{
		AccountInserter_.bindValue (":account_id", id);
		if (!AccountInserter_.exec ())
		{
			Util::DBLock::DumpError (AccountInserter_);
			return;
		}
		AccountInserter_.finish ();

		try
		{
			Accounts_ [id] = GetAccountID (id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to refetch id for"
					<< id
					<< "with:"
					<< e.what ();
		}
	}

	namespace
	{
		std::shared_ptr<void> CleanupQueryGuard (QSqlQuery& query)
		{
			return std::shared_ptr<void> (static_cast<void*> (0), [&query] (void*) { query.finish (); });
		}
	}

	Storage::RawSearchResult Storage::Search (const QString& accountId, const QString& entryId, const QString& text, int shift)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents"
					<< Accounts_;
			return RawSearchResult ();
		}
		if (!Users_.contains (entryId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Users_ doesn't contain"
					<< entryId
					<< "; raw contents"
					<< Users_;
			return RawSearchResult ();
		}

		const qint32 intEntryId = Users_ [entryId];
		const qint32 intAccId = Accounts_ [accountId];
		LogsSearcher_.bindValue (":entry_id", intEntryId);
		LogsSearcher_.bindValue (":account_id", intAccId);
		LogsSearcher_.bindValue (":inner_entry_id", intEntryId);
		LogsSearcher_.bindValue (":inner_account_id", intAccId);
		LogsSearcher_.bindValue (":text", '%' + text + '%');
		LogsSearcher_.bindValue (":offset", shift);
		if (!LogsSearcher_.exec ())
		{
			Util::DBLock::DumpError (LogsSearcher_);
			return RawSearchResult ();
		}
		auto guard = CleanupQueryGuard (LogsSearcher_);

		if (!LogsSearcher_.next ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to move to the next entry";
			return RawSearchResult ();
		}

		return RawSearchResult (intEntryId, intAccId, LogsSearcher_.value (0).toDateTime ());
	}

	Storage::RawSearchResult Storage::Search (const QString& accountId, const QString& text, int shift)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents"
					<< Accounts_;
			return RawSearchResult ();
		}

		const qint32 intAccId = Accounts_ [accountId];
		LogsSearcherWOContact_.bindValue (":account_id", intAccId);
		LogsSearcherWOContact_.bindValue (":inner_account_id", intAccId);
		LogsSearcherWOContact_.bindValue (":text", '%' + text + '%');
		LogsSearcherWOContact_.bindValue (":offset", shift);
		if (!LogsSearcherWOContact_.exec ())
		{
			Util::DBLock::DumpError (LogsSearcherWOContact_);
			return RawSearchResult ();
		}

		if (!LogsSearcherWOContact_.next ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to move to the next entry";
			return RawSearchResult ();
		}

		auto guard = CleanupQueryGuard (LogsSearcherWOContact_);

		return RawSearchResult (LogsSearcherWOContact_.value (1).toInt (),
				intAccId,
				LogsSearcherWOContact_.value (0).toDateTime ());
	}

	Storage::RawSearchResult Storage::Search (const QString& text, int shift)
	{
		LogsSearcherWOContactAccount_.bindValue (":text", '%' + text + '%');
		LogsSearcherWOContactAccount_.bindValue (":offset", shift);
		if (!LogsSearcherWOContactAccount_.exec ())
		{
			Util::DBLock::DumpError (LogsSearcherWOContactAccount_);
			return RawSearchResult ();
		}

		if (!LogsSearcherWOContactAccount_.next ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to move to the next entry";
			return RawSearchResult ();
		}

		auto guard = CleanupQueryGuard (LogsSearcherWOContactAccount_);

		return RawSearchResult (LogsSearcherWOContactAccount_.value (1).toInt (),
				LogsSearcherWOContactAccount_.value (2).toInt (),
				LogsSearcherWOContactAccount_.value (0).toDateTime ());
	}

	void Storage::addMessage (const QVariantMap& data)
	{
		Util::DBLock lock (*DB_);
		try
		{
			lock.Init ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to start transaction:"
					<< e.what ();
			return;
		}

		const QString& entryID = data ["EntryID"].toString ();

		if (!Users_.contains (entryID))
		{
			try
			{
				AddUser (entryID);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< entryID
						<< "unable to add the user to the DB:"
						<< e.what ();
				return;
			}
		}

		EntryCacheSetter_.bindValue (":id", Users_ [entryID]);
		EntryCacheSetter_.bindValue (":visible_name", data ["VisibleName"]);
		if (!EntryCacheSetter_.exec ())
			Util::DBLock::DumpError (EntryCacheSetter_);

		const QString& accountID = data ["AccountID"].toString ();
		if (!Accounts_.contains (accountID))
		{
			try
			{
				AddAccount (accountID);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< accountID
						<< "unable to add account ID to the DB:"
						<< e.what ();
				return;
			}
		}

		MessageDumper_.bindValue (":id", Users_ [entryID]);
		MessageDumper_.bindValue (":account_id", Accounts_ [accountID]);
		MessageDumper_.bindValue (":date", data ["DateTime"]);
		MessageDumper_.bindValue (":direction", data ["Direction"]);
		MessageDumper_.bindValue (":message", data ["Body"]);
		MessageDumper_.bindValue (":variant", data ["OtherVariant"]);

		switch (data ["MessageType"].toInt ())
		{
		case IMessage::MTChatMessage:
			MessageDumper_.bindValue (":type", "CHAT");
			break;
		case IMessage::MTMUCMessage:
			MessageDumper_.bindValue (":type", "MUC");
			break;
		case IMessage::MTStatusMessage:
			MessageDumper_.bindValue (":type", "STATUS");
			break;
		case IMessage::MTEventMessage:
			MessageDumper_.bindValue (":type", "EVENT");
			break;
		case IMessage::MTServiceMessage:
			MessageDumper_.bindValue (":type", "SERVICE");
			break;
		}

		if (!MessageDumper_.exec ())
		{
			Util::DBLock::DumpError (MessageDumper_);
			return;
		}

		lock.Good ();
	}

	void Storage::getOurAccounts ()
	{
		emit gotOurAccounts (Accounts_.keys ());
	}

	void Storage::getUsersForAccount (const QString& accountId)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents:"
					<< Accounts_;
			return;
		}

		UsersForAccountGetter_.bindValue (":account_id", Accounts_ [accountId]);
		if (!UsersForAccountGetter_.exec ())
		{
			Util::DBLock::DumpError (UsersForAccountGetter_);
			return;
		}

		QStringList result;
		QStringList cachedNames;
		while (UsersForAccountGetter_.next ())
		{
			const int id = UsersForAccountGetter_.value (0).toInt ();
			result << UsersForAccountGetter_.value (1).toString ();

			EntryCacheGetter_.bindValue (":id", id);
			if (!EntryCacheGetter_.exec ())
				Util::DBLock::DumpError (EntryCacheGetter_);

			EntryCacheGetter_.next ();
			cachedNames << EntryCacheGetter_.value (0).toString ();
		}
		EntryCacheGetter_.finish ();

		emit gotUsersForAccount (result, accountId, cachedNames);
	}

	void Storage::getChatLogs (const QString& accountId,
			const QString& entryId, int backpages, int amount)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents"
					<< Accounts_;
			return;
		}
		if (!Users_.contains (entryId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Users_ doesn't contain"
					<< entryId
					<< "; raw contents"
					<< Users_;
			return;
		}

		HistoryGetter_.bindValue (":entry_id", Users_ [entryId]);
		HistoryGetter_.bindValue (":account_id", Accounts_ [accountId]);
		HistoryGetter_.bindValue (":limit", amount);
		HistoryGetter_.bindValue (":offset", amount * backpages);

		if (!HistoryGetter_.exec ())
		{
			Util::DBLock::DumpError (HistoryGetter_);
			return;
		}

		QList<QVariant> result;
		while (HistoryGetter_.next ())
		{
			QVariantMap map;
			map ["Date"] = HistoryGetter_.value (0);
			map ["Direction"] = HistoryGetter_.value (1);
			map ["Message"] = HistoryGetter_.value (2);
			map ["Variant"] = HistoryGetter_.value (3);
			map ["Type"] = HistoryGetter_.value (4);
			result.prepend (map);
		}

		emit gotChatLogs (accountId, entryId, backpages, amount, result);
	}

	void Storage::search (const QString& accountId,
			const QString& entryId, const QString& text, int shift)
	{
		RawSearchResult res;
		if (!accountId.isEmpty () && !entryId.isEmpty ())
			res = Search (accountId, entryId, text, shift);
		else if (!accountId.isEmpty ())
			res = Search (accountId, text, shift);
		else
			res = Search (text, shift);

		if (res.Date_.isNull ())
		{
			emit gotSearchPosition (accountId, entryId, 0);
			return;
		}

		if (res.IsEmpty ())
			return;

		Date2Pos_.bindValue (":date", res.Date_);
		Date2Pos_.bindValue (":account_id", res.AccountID_);
		Date2Pos_.bindValue (":entry_id", res.EntryID_);
		if (!Date2Pos_.exec ())
		{
			Util::DBLock::DumpError (Date2Pos_);
			return;
		}

		if (!Date2Pos_.next ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to navigate to next record";
			return;
		}

		const int index = Date2Pos_.value (0).toInt ();
		Date2Pos_.finish ();

		emit gotSearchPosition (Accounts_.key (res.AccountID_), Users_.key (res.EntryID_), index);
	}

	void Storage::clearHistory (const QString& accountId, const QString& entryId)
	{
		if (!Accounts_.contains (accountId) ||
				!Users_.contains (entryId))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown entry/account combination"
					<< accountId
					<< entryId;
			return;
		}
		HistoryClearer_.bindValue (":entry_id", Users_ [entryId]);
		HistoryClearer_.bindValue (":account_id", Accounts_ [accountId]);

		if (!HistoryClearer_.exec ())
			Util::DBLock::DumpError (HistoryClearer_);
	}
}
}
}
