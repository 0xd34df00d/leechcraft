/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <plugininterface/util.h>
#include <plugininterface/dblock.h>
#include <interfaces/iclentry.h>
#include <boost/graph/graph_concepts.hpp>
#include <interfaces/iaccount.h>

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
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

		if (!DB_->tables ().contains ("azoth_history"))
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
		UsersForAccountGetter_.prepare ("SELECT DISTINCT EntryID FROM azoth_users, azoth_history "
				"WHERE azoth_history.Id = azoth_users.Id AND AccountID = :account_id;");
		HistoryGetter_ = QSqlQuery (*DB_);
		HistoryGetter_.prepare ("SELECT Date, Direction, Message, Variant, Type "
				"FROM azoth_history "
				"WHERE Id = :entry_id "
				"AND AccountID = :account_id "
				"ORDER BY Date DESC LIMIT :limit OFFSET :offset;");
		HistoryClearer_ = QSqlQuery (*DB_);
		HistoryClearer_.prepare ("DELETE FROM azoth_history WHERE Id = :entry_id AND AccountID = :account_id;");
		
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
		QStringList queries;
		queries << "CREATE TABLE azoth_users ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"EntryID TEXT "
					");"
				<< "CREATE TABLE azoth_accounts ("
					"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"AccountID TEXT "
					");"
				<< "CREATE TABLE azoth_history ("
					"Id INTEGER, "
					"AccountId INTEGER, "
					"Date DATETIME, "
					"Direction INTEGER, "
					"Message TEXT, "
					"Variant TEXT, "
					"Type INTEGER, "
					"UNIQUE (Id, AccountId, Date, Direction, Message, Variant, Type) ON CONFLICT IGNORE);";
		Q_FOREACH (const QString& queryStr, queries)
			if (!query.exec (queryStr))
			{
				Util::DBLock::DumpError (query);
				throw std::runtime_error ("Unable to create tables for Azoth history");
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
	
	void Storage::addMessage (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (msg->GetBody ().isEmpty ())
			return;
		
		if (msg->GetDirection () == IMessage::DOut &&
				msg->GetMessageType () == IMessage::MTMUCMessage)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "message's other part doesn't implement ICLEntry"
					<< msg->GetObject ()
					<< msg->OtherPart ();
			return;
		}
		
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
		
		const QString& entryID = entry->GetEntryID ();

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

		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
		const QString& accountID = account->GetAccountID ();
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
		MessageDumper_.bindValue (":date", msg->GetDateTime ());
		MessageDumper_.bindValue (":direction",
				msg->GetDirection () == IMessage::DIn ? "IN" : "OUT");
		MessageDumper_.bindValue (":message", msg->GetBody ());
		MessageDumper_.bindValue (":variant", msg->GetOtherVariant ());

		switch (msg->GetMessageType ())
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
		while (UsersForAccountGetter_.next ())
			result << UsersForAccountGetter_.value (0).toString ();
		
		qDebug () << accountId << Accounts_ << result;
		
		emit gotUsersForAccount (result, accountId);
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
