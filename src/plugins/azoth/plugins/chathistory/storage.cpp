/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storage.h"
#include <algorithm>
#include <stdexcept>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/sys/paths.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <util/sll/unreachable.h>
#include <util/util.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	Storage::RawSearchResult::RawSearchResult (qint32 entryId, qint32 accountId, qint64 rowId)
	: EntryID_ { entryId }
	, AccountID_ { accountId }
	, RowID_ { rowId }
	{
	}

	bool Storage::RawSearchResult::IsEmpty () const
	{
		return RowID_ < 0 || !EntryID_ || !AccountID_;
	}

	Storage::Storage (QObject *parent)
	: QObject (parent)
	, DB_ (std::make_shared<QSqlDatabase> (QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("Azoth.ChatHistory.HistoryConnection"))))
	{
		DB_->setDatabaseName (GetDatabasePath ());
	}

	QString Storage::GetDatabasePath ()
	{
		return Util::CreateIfNotExists ("azoth").filePath ("history.db");
	}

	QSqlDatabase Storage::GetDB () const
	{
		return *DB_;
	}

	Storage::InitializationResult_t Storage::Initialize ()
	{
		if (DB_->isOpen ())
			DB_->close ();

		if (!DB_->open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (DB_->lastError ());
			return Util::Left { GeneralError { tr ("Unable to open Azoth history database.") } };
		}

		QSqlQuery pragma (*DB_);
		pragma.exec ("PRAGMA foreign_keys = ON;");
		pragma.exec ("PRAGMA synchronous = OFF");

		InitializeTables ();

		MaxTimestampSelector_ = QSqlQuery (*DB_);
		MaxTimestampSelector_.prepare ("SELECT max(Date) FROM azoth_history WHERE AccountID = :account_id");

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
		MessageDumper_.prepare ("INSERT INTO azoth_history (Id, AccountID, Date, Direction, Message, Variant, Type, RichMessage, EscapePolicy) "
				"VALUES (:id, :account_id, :date, :direction, :message, :variant, :type, :rich_message, :escape_policy);");

		MessageDumperFuzzy_ = QSqlQuery (*DB_);
		MessageDumperFuzzy_.prepare (R"(
				INSERT INTO azoth_history (Id, AccountID, Date, Direction, Message, Variant, Type, RichMessage, EscapePolicy)
				SELECT :id, :account_id, :date, :direction, :message, :variant, :type, :rich_message, :escape_policy
				WHERE NOT EXISTS (
					SELECT 1 FROM azoth_history
					WHERE Id = :id_inner
						AND AccountID = :account_id_inner
						AND Direction = :direction_inner
						AND Message = :message_inner
						AND abs(Date - :date_inner) < :tolerance
				)
				)");

		UsersForAccountGetter_ = QSqlQuery (*DB_);
		UsersForAccountGetter_.prepare ("SELECT DISTINCT azoth_acc2users2.UserId, EntryID FROM azoth_users, azoth_acc2users2 "
				"WHERE azoth_acc2users2.UserId = azoth_users.Id AND azoth_acc2users2.AccountID = :account_id;");

		RowID2Pos_ = QSqlQuery (*DB_);
		RowID2Pos_.prepare ("SELECT COUNT(1) FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"AND rowid > :rowid");

		Date2Pos_ = QSqlQuery (*DB_);
		Date2Pos_.prepare ("SELECT COUNT(1) FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"AND Date >= :date");

		GetMonthDates_ = QSqlQuery (*DB_);
		GetMonthDates_.prepare ("SELECT Date FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"AND Date >= :lower_date "
				"AND Date <= :upper_date");

		LogsSearcher_ = QSqlQuery (*DB_);
		LogsSearcher_.prepare ("SELECT Rowid FROM azoth_history "
				"WHERE Id = :inner_entry_id "
				"AND AccountID = :inner_account_id "
				"AND ((Message LIKE :text AND :insensitive) OR (Message GLOB :ctext AND :sensitive)) "
				"ORDER BY Rowid DESC "
				"LIMIT 1 OFFSET :offset;");

		LogsSearcherWOContact_ = QSqlQuery (*DB_);
		LogsSearcherWOContact_.prepare ("SELECT Date, Id FROM azoth_history "
				"WHERE AccountID = :account_id "
				"AND Date = (SELECT Date FROM azoth_history "
				"	WHERE AccountID = :inner_account_id "
				"	AND ((Message LIKE :text AND :insensitive) OR (Message GLOB :ctext AND :sensitive)) "
				"	ORDER BY Rowid DESC "
				"	LIMIT 1 OFFSET :offset);");

		LogsSearcherWOContactAccount_ = QSqlQuery (*DB_);
		LogsSearcherWOContactAccount_.prepare ("SELECT Date, Id, AccountID FROM azoth_history "
				"WHERE Date = (SELECT Date FROM azoth_history "
				"	WHERE ((Message LIKE :text AND :insensitive) OR (Message GLOB :ctext AND :sensitive)) "
				"	ORDER BY Rowid DESC "
				"	LIMIT 1 OFFSET :offset);");

		HistoryGetter_ = QSqlQuery (*DB_);
		HistoryGetter_.prepare ("SELECT Date, Direction, Message, Variant, Type, RichMessage, EscapePolicy "
				"FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"ORDER BY Rowid DESC LIMIT :limit OFFSET :offset;");

		HistoryClearer_ = QSqlQuery (*DB_);
		HistoryClearer_.prepare ("DELETE FROM azoth_history WHERE Id = :entry_id AND AccountID = :account_id;");

		UserClearer_ = QSqlQuery (*DB_);
		UserClearer_.prepare ("DELETE FROM azoth_users WHERE Id = :user_id;");

		EntryCacheGetter_ = QSqlQuery (*DB_);
		EntryCacheGetter_.prepare ("SELECT Id, VisibleName FROM azoth_entrycache;");

		EntryCacheSetter_ = QSqlQuery (*DB_);
		EntryCacheSetter_.prepare ("INSERT OR REPLACE INTO azoth_entrycache (Id, VisibleName) "
				"VALUES (:id, :visible_name);");

		EntryCacheClearer_ = QSqlQuery (*DB_);
		EntryCacheClearer_.prepare ("DELETE FROM azoth_entrycache WHERE Id = :user_id;");

		try
		{
			Users_ = GetUsers ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get saved users, we would be a bit more inefficient"
					<< e.what ();
		}

		try
		{
			Accounts_ = GetAccounts ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get saved accounts, we would be a bit more inefficient"
					<< e.what ();
		}

		PrepareEntryCache ();

		return Util::Void {};
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
		const QList<QPair<QString, QString>> table2query
		{
			{
				"azoth_users",
				"CREATE TABLE azoth_users ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"EntryID TEXT "
					");"
			},
			{
				"azoth_accounts",
				"CREATE TABLE azoth_accounts ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"AccountID TEXT "
					");"
			},
			{
				"azoth_history",
				"CREATE TABLE azoth_history ("
					"Id INTEGER, "
					"AccountId INTEGER, "
					"Date DATETIME, "
					"Direction INTEGER, "
					"Message TEXT, "
					"Variant TEXT, "
					"Type INTEGER, "
					"RichMessage TEXT, "
					"EscapePolicy VARCHAR(3), "
					"UNIQUE (Id, AccountId, Date, Direction, Message, Variant, Type) ON CONFLICT IGNORE);"
			},
			{
				"azoth_entrycache",
				"CREATE TABLE azoth_entrycache ("
					"Id INTEGER UNIQUE ON CONFLICT REPLACE REFERENCES azoth_users (Id) ON DELETE CASCADE, "
					"VisibleName TEXT "
					");"
			},
			{
				"azoth_acc2users2",
				"CREATE TABLE azoth_acc2users2 ("
					"AccountId INTEGER REFERENCES azoth_accounts (Id) ON DELETE CASCADE, "
					"UserId INTEGER REFERENCES azoth_users (Id) ON DELETE CASCADE, "
					"UNIQUE (AccountId, UserId)"
					");"
			}
		};

		const auto& tables = DB_->tables ();
		const bool hadAcc2User = tables.contains ("azoth_acc2users2");

		if (tables.contains ("azoth_acc2users"))
			query.exec ("DROP TABLE azoth_acc2users;");

		for (const auto& pair : table2query)
		{
			if (tables.contains (pair.first))
				continue;

			const auto& queryStr = pair.second;
			if (!query.exec (queryStr))
			{
				Util::DBLock::DumpError (query);
				throw std::runtime_error ("Unable to create tables for Azoth history");
			}
		}

		UpdateTables ();

		if (!query.exec ("CREATE INDEX IF NOT EXISTS azoth_history_id_accountid ON azoth_history (Id, AccountId);"))
		{
			Util::DBLock::DumpError (query);
			throw std::runtime_error ("Unable to index `azoth_history`.");
		}

		if (!hadAcc2User)
			RegenUsersCache ();

		lock.Good ();
	}

	void Storage::UpdateTables ()
	{
		QSqlQuery query { *DB_ };

		if (!query.exec ("PRAGMA table_info (azoth_history);"))
		{
			Util::DBLock::DumpError (query);
			throw std::runtime_error ("Unable to get table information about `azoth_history`.");
		}

		QSet<QString> columns;
		while (query.next ())
			columns << query.value (1).toString ();

		if (!columns.contains ("RichMessage") &&
				!query.exec ("ALTER TABLE azoth_history ADD COLUMN RichMessage TEXT;"))
		{
			Util::DBLock::DumpError (query);
			qWarning () << Q_FUNC_INFO
					<< "existing columns:"
					<< columns;
			throw std::runtime_error ("Unable to add column `RichMessage` to `azoth_history`.");
		}

		if (!columns.contains ("EscapePolicy") &&
				!query.exec ("ALTER TABLE azoth_history ADD COLUMN EscapePolicy VARCHAR(3);"))
		{
			Util::DBLock::DumpError (query);
			qWarning () << Q_FUNC_INFO
					<< "existing columns:"
					<< columns;
			throw std::runtime_error ("Unable to add column `EscapePolicy` to `azoth_history`.");
		}
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

	void Storage::AddUser (const QString& id, const QString& accountId)
	{
		UserInserter_.bindValue (":entry_id", id);
		if (!UserInserter_.exec ())
		{
			Util::DBLock::DumpError (UserInserter_);
			return;
		}
		UserInserter_.finish ();

		qint32 numericId = 0;
		try
		{
			numericId = GetUserID (id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to refetch id for"
					<< id
					<< "with:"
					<< e.what ();
			return;
		}

		Users_ [id] = numericId;

		QSqlQuery acc2users (*DB_);
		acc2users.prepare ("INSERT INTO azoth_acc2users2 (AccountId, UserId) VALUES (:accId, :userId);");
		acc2users.bindValue (":accId", Accounts_ [accountId]);
		acc2users.bindValue (":userId", numericId);
		if (!acc2users.exec ())
			Util::DBLock::DumpError (UserInserter_);
	}

	void Storage::PrepareEntryCache ()
	{
		if (!EntryCacheGetter_.exec ())
		{
			Util::DBLock::DumpError (EntryCacheGetter_);
			return;
		}

		while (EntryCacheGetter_.next ())
			EntryCache_ [EntryCacheGetter_.value (0).toInt ()] = EntryCacheGetter_.value (1).toString ();

		EntryCacheGetter_.finish ();

		qDebug () << Q_FUNC_INFO << "loaded" << EntryCache_.size () << "entries";
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
		auto CleanupQueryGuard (QSqlQuery& query)
		{
			return Util::MakeScopeGuard ([&query] { query.finish (); });
		}
	}

	Storage::RawSearchResult Storage::SearchImpl (const QString& accountId,
			const QString& entryId, const QString& text, int shift, bool cs)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents"
					<< Accounts_;
			return {};
		}
		if (!Users_.contains (entryId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Users_ doesn't contain"
					<< entryId
					<< "; raw contents"
					<< Users_;
			return {};
		}

		const qint32 intEntryId = Users_ [entryId];
		const qint32 intAccId = Accounts_ [accountId];
		LogsSearcher_.bindValue (":entry_id", intEntryId);
		LogsSearcher_.bindValue (":account_id", intAccId);
		LogsSearcher_.bindValue (":inner_entry_id", intEntryId);
		LogsSearcher_.bindValue (":inner_account_id", intAccId);
		LogsSearcher_.bindValue (":text", '%' + text + '%');
		LogsSearcher_.bindValue (":ctext", '*' + text + '*');
		LogsSearcher_.bindValue (":sensitive", static_cast<int> (cs));
		LogsSearcher_.bindValue (":insensitive", static_cast<int> (!cs));
		LogsSearcher_.bindValue (":offset", shift);
		if (!LogsSearcher_.exec ())
		{
			Util::DBLock::DumpError (LogsSearcher_);
			return {};
		}
		auto guard = CleanupQueryGuard (LogsSearcher_);

		if (!LogsSearcher_.next ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to move to the next entry";
			Util::DBLock::DumpError (LogsSearcher_);
			return {};
		}

		return { intEntryId, intAccId, LogsSearcher_.value (0).value<qint64> () };
	}

	Storage::RawSearchResult Storage::SearchImpl (const QString& accountId,
			const QString& text, int shift, bool cs)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents"
					<< Accounts_;
			return {};
		}

		const qint32 intAccId = Accounts_ [accountId];
		LogsSearcherWOContact_.bindValue (":account_id", intAccId);
		LogsSearcherWOContact_.bindValue (":inner_account_id", intAccId);
		LogsSearcherWOContact_.bindValue (":text", '%' + text + '%');
		LogsSearcherWOContact_.bindValue (":ctext", '*' + text + '*');
		LogsSearcherWOContact_.bindValue (":sensitive", static_cast<int> (cs));
		LogsSearcherWOContact_.bindValue (":insensitive", static_cast<int> (!cs));
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

		return
		{
			LogsSearcherWOContact_.value (1).toInt (),
			intAccId,
			LogsSearcherWOContact_.value (0).value<qint64> ()
		};
	}

	Storage::RawSearchResult Storage::SearchImpl (const QString& text, int shift, bool cs)
	{
		LogsSearcherWOContactAccount_.bindValue (":text", '%' + text + '%');
		LogsSearcherWOContactAccount_.bindValue (":ctext", '*' + text + '*');
		LogsSearcherWOContactAccount_.bindValue (":sensitive", static_cast<int> (cs));
		LogsSearcherWOContactAccount_.bindValue (":insensitive", static_cast<int> (!cs));
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

		return
		{
			LogsSearcherWOContactAccount_.value (1).toInt (),
			LogsSearcherWOContactAccount_.value (2).toInt (),
			LogsSearcherWOContactAccount_.value (0).value<qint64> ()
		};
	}

	SearchResult_t Storage::SearchRowIdImpl (qint32 accountId, qint32 entryId, qint64 rowId)
	{
		RowID2Pos_.bindValue (":rowid", rowId);
		RowID2Pos_.bindValue (":account_id", accountId);
		RowID2Pos_.bindValue (":entry_id", entryId);
		if (!RowID2Pos_.exec ())
		{
			Util::DBLock::DumpError (RowID2Pos_);
			return Util::Left { "Unable to execute search query." };
		}

		if (!RowID2Pos_.next ())
			return { std::nullopt };

		const int index = RowID2Pos_.value (0).toInt ();
		RowID2Pos_.finish ();

		return { index };
	}

	SearchResult_t Storage::SearchDateImpl (qint32 accountId, qint32 entryId, const QDateTime& dt)
	{
		Date2Pos_.bindValue (":date", dt);
		Date2Pos_.bindValue (":account_id", accountId);
		Date2Pos_.bindValue (":entry_id", entryId);
		if (!Date2Pos_.exec ())
		{
			Util::DBLock::DumpError (Date2Pos_);
			return Util::Left { "Unable to execute search query." };
		}

		if (!Date2Pos_.next ())
		{
			qWarning () << "unable to navigate to next record";
			return Util::Left { "Unable to navigate to the search results." };
		}

		const int index = Date2Pos_.value (0).toInt ();
		Date2Pos_.finish ();

		return { index };
	}

	std::optional<int> Storage::GetAllHistoryCount ()
	{
		QSqlQuery query { *DB_ };
		if (!query.exec ("SELECT COUNT(1) FROM azoth_history"))
		{
			Util::DBLock::DumpError (query);
			return {};
		}

		if (!query.next ())
		{
			qWarning () << "unable to navigate to next record";
			return {};
		}

		return query.value (0).toInt ();
	}

	void Storage::RegenUsersCache ()
	{
		QSqlQuery query (*DB_);
		if (!query.exec ("DELETE FROM azoth_acc2users2;") ||
			!query.exec ("INSERT INTO azoth_acc2users2 (AccountId, UserId) SELECT DISTINCT AccountId, Id FROM azoth_history;"))
		{
			Util::DBLock::DumpError (query);
			query.exec ("DROP TABLE azoth_acc2users2");
		}
	}

	namespace
	{
		QVariant ToVariant (IMessage::Direction dir)
		{
			switch (dir)
			{
			case IMessage::Direction::In:
				return "IN";
			case IMessage::Direction::Out:
				return "OUT";
			}

			Util::Unreachable ();
		}

		QVariant ToVariant (IMessage::EscapePolicy escPolicy)
		{
			switch (escPolicy)
			{
			case IMessage::EscapePolicy::Escape:
				return "Esc";
			case IMessage::EscapePolicy::NoEscape:
				return "NEs";
			}

			Util::Unreachable ();
		}

		QVariant ToVariant (IMessage::Type type)
		{
			switch (type)
			{
			case IMessage::Type::ChatMessage:
				return "CHAT";
			case IMessage::Type::MUCMessage:
				return "MUC";
			case IMessage::Type::StatusMessage:
				return "STATUS";
			case IMessage::Type::EventMessage:
				return "EVENT";
			case IMessage::Type::ServiceMessage:
				return "SERVICE";
			}

			Util::Unreachable ();
		}
	}

	namespace
	{
		void BindStrict (QSqlQuery& dumper, qint32 userId, qint32 accountId, const LogItem& logItem)
		{
			dumper.bindValue (":id", userId);
			dumper.bindValue (":account_id", accountId);
			dumper.bindValue (":date", logItem.Date_);
			dumper.bindValue (":direction", ToVariant (logItem.Dir_));
			dumper.bindValue (":message", logItem.Message_);
			dumper.bindValue (":variant", logItem.Variant_);
			dumper.bindValue (":rich_message", logItem.RichMessage_);
			dumper.bindValue (":escape_policy", ToVariant (logItem.EscPolicy_));
			dumper.bindValue (":type", ToVariant (logItem.Type_));
		}

		void BindFuzzy (QSqlQuery& dumper, qint32 userId, qint32 accountId, const LogItem& logItem)
		{
			BindStrict (dumper, userId, accountId, logItem);

			dumper.bindValue (":id_inner", userId);
			dumper.bindValue (":account_id_inner", accountId);
			dumper.bindValue (":date_inner", logItem.Date_);
			dumper.bindValue (":direction_inner", ToVariant (logItem.Dir_));
			dumper.bindValue (":message_inner", logItem.Message_);
			dumper.bindValue (":tolerance", 0.1);
		}
	}

	void Storage::AddMessages (const QString& accountID,
			const QString& entryID, const QString& visibleName,
			const QList<LogItem>& items, bool fuzzy)
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

		if (!Accounts_.contains (accountID))
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

		if (!Users_.contains (entryID))
			try
			{
				AddUser (entryID, accountID);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< entryID
						<< "unable to add the user to the DB:"
						<< e.what ();
				return;
			}

		auto userId = Users_ [entryID];
		if (!EntryCache_.contains (userId))
		{
			EntryCacheSetter_.bindValue (":id", userId);
			EntryCacheSetter_.bindValue (":visible_name", visibleName);
			if (!EntryCacheSetter_.exec ())
				Util::DBLock::DumpError (EntryCacheSetter_);

			EntryCache_ [userId] = visibleName;
		}

		for (const auto& logItem : items)
		{
			auto& query = fuzzy ? MessageDumperFuzzy_ : MessageDumper_;

			if (fuzzy)
				BindFuzzy (query, userId, Accounts_ [accountID], logItem);
			else
				BindStrict (query, userId, Accounts_ [accountID], logItem);

			if (!query.exec ())
			{
				Util::DBLock::DumpError (query);
				return;
			}
		}

		lock.Good ();
	}

	IHistoryPlugin::MaxTimestampResult_t Storage::GetMaxTimestamp (const QString& accountId)
	{
		if (!Accounts_.contains (accountId))
			return Util::Left { "Unknown account " + accountId };

		MaxTimestampSelector_.bindValue (":account_id", Accounts_ [accountId]);
		if (!MaxTimestampSelector_.exec ())
		{
			Util::DBLock::DumpError (MaxTimestampSelector_);
			return Util::Left { "Error executing the SQL query." };
		}

		if (!MaxTimestampSelector_.next ())
			return { QDateTime {} };

		return { MaxTimestampSelector_.value (0).toDateTime () };
	}

	QStringList Storage::GetOurAccounts () const
	{
		return Accounts_.keys ();
	}

	UsersForAccountResult_t Storage::GetUsersForAccount (const QString& accountId)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents:"
					<< Accounts_;
			return Util::Left { "Unknown account." };
		}

		UsersForAccountGetter_.bindValue (":account_id", Accounts_ [accountId]);
		if (!UsersForAccountGetter_.exec ())
		{
			Util::DBLock::DumpError (UsersForAccountGetter_);
			return Util::Left { "Error executing the SQL query." };
		}

		QStringList result;
		QStringList cachedNames;
		while (UsersForAccountGetter_.next ())
		{
			const int id = UsersForAccountGetter_.value (0).toInt ();
			result << UsersForAccountGetter_.value (1).toString ();
			cachedNames << EntryCache_.value (id);
		}

		return { { result, cachedNames } };
	}

	namespace
	{
		IMessage::Direction GetMsgDirection (const QVariant& var)
		{
			return var.toString () == "IN" ?
					IMessage::Direction::In :
					IMessage::Direction::Out;
		}

		IMessage::Type GetMsgType (const QVariant& var)
		{
			return var.toString () == "CHAT" ?
					IMessage::Type::ChatMessage :
					IMessage::Type::MUCMessage;
		}

		IMessage::EscapePolicy GetMsgEscapePolicy (const QVariant& var)
		{
			return var.toString () == "NEs" ?
					IMessage::EscapePolicy::NoEscape :
					IMessage::EscapePolicy::Escape;
		}
	}

	ChatLogsResult_t Storage::GetChatLogs (const QString& accountId,
			const QString& entryId, int backpages, int amount)
	{
		if (!Accounts_.contains (accountId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< accountId
					<< "; raw contents"
					<< Accounts_;
			return Util::Left { "Unknown account." };
		}
		if (!Users_.contains (entryId))
		{
			qWarning () << Q_FUNC_INFO
					<< "Users_ doesn't contain"
					<< entryId
					<< "; raw contents"
					<< Users_;
			return Util::Left { "Unknown user." };
		}

		HistoryGetter_.bindValue (":entry_id", Users_ [entryId]);
		HistoryGetter_.bindValue (":account_id", Accounts_ [accountId]);
		HistoryGetter_.bindValue (":limit", amount);
		HistoryGetter_.bindValue (":offset", amount * backpages);

		if (!HistoryGetter_.exec ())
		{
			Util::DBLock::DumpError (HistoryGetter_);
			return Util::Left { "Unable to execute the SQL query." };
		}

		LogList_t result;
		while (HistoryGetter_.next ())
			result.push_back ({
					HistoryGetter_.value (0).toDateTime (),
					GetMsgDirection (HistoryGetter_.value (1)),
					HistoryGetter_.value (2).toString (),
					HistoryGetter_.value (3).toString (),
					GetMsgType (HistoryGetter_.value (4)),
					HistoryGetter_.value (5).toString (),
					GetMsgEscapePolicy (HistoryGetter_.value (6))
				});

		std::reverse (result.begin (), result.end ());

		return { result };
	}

	SearchResult_t Storage::Search (const QString& accountId,
			const QString& entryId, const QString& text, int shift, bool cs)
	{
		RawSearchResult res;
		if (!accountId.isEmpty () && !entryId.isEmpty ())
			res = SearchImpl (accountId, entryId, text, shift, cs);
		else if (!accountId.isEmpty ())
			res = SearchImpl (accountId, text, shift, cs);
		else
			res = SearchImpl (text, shift, cs);

		if (res.IsEmpty ())
			return { std::nullopt };

		return SearchRowIdImpl (res.AccountID_, res.EntryID_, res.RowID_);
	}

	SearchResult_t Storage::SearchDate (const QString& account, const QString& entry, const QDateTime& dt)
	{
		if (!Accounts_.contains (account))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< account
					<< "; raw contents"
					<< Accounts_;
			return Util::Left { "Unknown account." };
		}
		if (!Users_.contains (entry))
		{
			qWarning () << Q_FUNC_INFO
					<< "Users_ doesn't contain"
					<< entry
					<< "; raw contents"
					<< Users_;
			return Util::Left { "Unknown user." };
		}

		const qint32 entryId = Users_ [entry];
		const qint32 accId = Accounts_ [account];
		return SearchDateImpl (accId, entryId, dt);
	}

	DaysResult_t Storage::GetDaysForSheet (const QString& account, const QString& entry, int year, int month)
	{
		if (!Accounts_.contains (account))
		{
			qWarning () << Q_FUNC_INFO
					<< "Accounts_ doesn't contain"
					<< account
					<< "; raw contents"
					<< Accounts_;
			return Util::Left { "Unknown account." };
		}
		if (!Users_.contains (entry))
		{
			qWarning () << Q_FUNC_INFO
					<< "Users_ doesn't contain"
					<< entry
					<< "; raw contents"
					<< Users_;
			return Util::Left { "Unknown user." };
		}

		const QDate lowerDate (year, month, 1);
		const QDateTime lowerBound (lowerDate, QTime (0, 0, 0));
		const QDateTime upperBound (QDate (year, month, lowerDate.daysInMonth ()), QTime (23, 59, 59));

		GetMonthDates_.bindValue (":entry_id", Users_ [entry]);
		GetMonthDates_.bindValue (":account_id", Accounts_ [account]);
		GetMonthDates_.bindValue (":lower_date", lowerBound);
		GetMonthDates_.bindValue (":upper_date", upperBound);

		if (!GetMonthDates_.exec ())
		{
			Util::DBLock::DumpError (GetMonthDates_);
			return Util::Left { "Unable to execute SQL query." };
		}

		QList<int> result;
		while (GetMonthDates_.next ())
		{
			const auto date = GetMonthDates_.value (0).toDate ();
			const int day = date.day ();
			if (!result.contains (day))
				result << day;
		}
		std::sort (result.begin (), result.end ());

		return { result };
	}

	void Storage::ClearHistory (const QString& accountId, const QString& entryId)
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

		Util::DBLock lock (*DB_);
		lock.Init ();

		const auto userId = Users_.take (entryId);
		HistoryClearer_.bindValue (":entry_id", userId);
		HistoryClearer_.bindValue (":account_id", Accounts_ [accountId]);

		if (!HistoryClearer_.exec ())
			Util::DBLock::DumpError (HistoryClearer_);

		EntryCacheClearer_.bindValue (":user_id", userId);
		if (!EntryCacheClearer_.exec ())
			Util::DBLock::DumpError (EntryCacheClearer_);

		UserClearer_.bindValue (":user_id", userId);
		if (!UserClearer_.exec ())
			Util::DBLock::DumpError (UserClearer_);

		lock.Good ();
	}
}
}
}
