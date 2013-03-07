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

#include "localstorage.h"
#include <stdexcept>
#include <QtDebug>
#include <QSqlError>
#include <util/util.h>
#include <util/dblock.h>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LocalStorage::LocalStorage (const QByteArray& id, QObject *parent)
	: QObject (parent)
	, MetidaDB_ (QSqlDatabase::addDatabase ("QSQLITE",
			QString ("%1_localstorage").arg (QString::fromUtf8 (id))))
	{
		MetidaDB_.setDatabaseName (Util::CreateIfNotExists ("blogique/metida")
				.filePath ("metida.db"));

		if (!MetidaDB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (MetidaDB_.lastError ());
			throw std::runtime_error ("unable to open Metida database");
		}

		{
			QSqlQuery query (MetidaDB_);
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
		Util::DBLock lock (MetidaDB_);
		lock.Init ();

		AddAccount_.bindValue (":account_id", QString::fromUtf8 (accounId));
		if (!AddAccount_.exec ())
		{
			Util::DBLock::DumpError (AddAccount_);
			throw std::runtime_error ("unable to add account");
		}

		lock.Good ();
	}

	void LocalStorage::RemoveAccount (const QByteArray& accounId)
	{
		Util::DBLock lock (MetidaDB_);
		lock.Init ();

		RemoveAccount_.bindValue (":account_id", QString::fromUtf8 (accounId));
		if (!RemoveAccount_.exec ())
		{
			Util::DBLock::DumpError (RemoveAccount_);
			throw std::runtime_error ("unable to remove account");
		}

		lock.Good ();
	}

	void LocalStorage::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["accounts"] = "CREATE TABLE IF NOT EXISTS accounts ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountID TEXT NOT NULL UNIQUE);";
		table2query ["inbox"] = "CREATE TABLE IF NOT EXISTS inbox ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE "
				"MessageId INTEGER NOT NULL UNIQUE, "
				"Type INTEGER NOT NULL, "
				"TypeString TEXT, "
				"Read BOOL,"
				"When DATE);";
		table2query ["extended"] = "CREATE TABLE IF NOT EXISTS inbox_extended ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE , "
				"ExtendedId INTEGER, "
				"ExtendedSubject TEXT, "
				"ExtendedText TEXT);";
		table2query ["new_comment"] = "CREATE TABLE IF NOT EXISTS inbox_new_comment ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE , "
				"Action TEXT, "
				"Poster TEXT, "
				"Journal TEXT, "
				"ReplyUrl TEXT, "
				"Subject TEXT, "
				"Url TEXT);";
		table2query ["received"] = "CREATE TABLE IF NOT EXISTS inbox_received ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE , "
				"Body TEXT, "
				"From TEXT, "
				"Journal TEXT, "
				"ReceivedMessageId INTEGER, "
				"ReceivedMessageParentId INTEGER, "
				"PictureUrl TEXT);";
		table2query ["sent"] = "CREATE TABLE IF NOT EXISTS inbox_sent ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE , "
				"Body TEXT, "
				"PictureUrl TEXT, "
				"To TEXT, "
				"Subject TEXT);";
		Util::DBLock lock (MetidaDB_);

		lock.Init ();

		const auto& tables = MetidaDB_.tables ();
		Q_FOREACH (const QString& key, table2query.keys ())
		if (!tables.contains (key))
		{
			QSqlQuery q (MetidaDB_);
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
		AddAccount_ = QSqlQuery (MetidaDB_);
		AddAccount_.prepare ("INSERT OR IGNORE INTO accounts (AccountID) "
				"VALUES (:account_id);");
		RemoveAccount_ = QSqlQuery (MetidaDB_);
		RemoveAccount_.prepare ("DELETE FROM accounts WHERE AccountID = :account_id;");

		AddMessage_ = QSqlQuery (MetidaDB_);
		AddMessage_.prepare ("INSERT OR IGNORE INTO inbox ("
				"AccountId, MessageId, Type, TypeString, Read, When) VALUES ("
				"(SELECTE Id FROM accounts WHERE (AccountId = :account_id), "
				":message_id, :type, :type_string, :read, :when);");
		RemoveMessage_ = QSqlQuery (MetidaDB_);
		RemoveMessage_.prepare ("DELETE FROM inbox WHERE MessageId = :message_id AND "
				"AccountId = (SELECTE Id FROM accounts WHERE (AccountId = :account_id);");
		UpdateMessage_ = QSqlQuery (MetidaDB_);
		UpdateMessage_.prepare ("UPDATE inbox SET Read = :read WHERE MessageId = :message_id AND "
				"AccountId = (SELECTE Id FROM accounts WHERE (AccountId = :account_id));");

		AddExtendedMessageParams_ = QSqlQuery (MetidaDB_);
		AddExtendedMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_extended ("
				"AccountId, MessageId, ExtendedId, ExtendedSubject, ExtendedText) "
				"VALUES (SELECTE Id FROM accounts WHERE AccountId = :account_id, "
				"SELECT Id FROM inbox WHERE MessageId = :message_id, :extended_id, "
				":extended_subject, :extended_text);");

		AddNewCommentMessageParams_ = QSqlQuery (MetidaDB_);
		AddNewCommentMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_new_comment ("
				"AccountId, MessageId, Action, Poster, Journal, ReplyUrl, Subject, Url) "
				"VALUES (SELECTE Id FROM accounts WHERE AccountId = :account_id, "
				"SELECT Id FROM inbox WHERE MessageId = :message_id, :action, "
				":poster, :journal, :reply_url, :subject, :url);");

		AddReceivedMessageParams_ = QSqlQuery (MetidaDB_);
		AddReceivedMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_received ("
				"AccountId, MessageId, Body, From, Journal, ReceivedMessageId, "
				"ReceivedMessageParentId, PictureUrl) "
				"VALUES (SELECTE Id FROM accounts WHERE AccountId = :account_id, "
				"SELECT Id FROM inbox WHERE MessageId = :message_id, :body, "
				":from, :journal, :recvd_id, :recvd_parent_id, :picture_url);");

		AddSentMessageParams_ = QSqlQuery (MetidaDB_);
		AddSentMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_sent ("
				"AccountId, MessageId, Body, To, Subject, PictureUrl) "
				"VALUES (SELECTE Id FROM accounts WHERE AccountId = :account_id, "
				"SELECT Id FROM inbox WHERE MessageId = :message_id, :body, "
				":to, :subject, :picture_url);");

		GetAllMessages_ = QSqlQuery (MetidaDB_);
		//TODO
		GetAllMessages_.prepare (QString ());

		GetMessage_ = QSqlQuery (MetidaDB_);
		//TODO
		GetMessage_.prepare (QString ());
	}

}
}
}