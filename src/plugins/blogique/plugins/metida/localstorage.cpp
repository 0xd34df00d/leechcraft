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

	void LocalStorage::AddMessage (LJInbox::Message *msg, const QByteArray& accounId)
	{
		AddMessage_.bindValue (":account_id", QString::fromUtf8 (accounId));
		AddMessage_.bindValue (":message_id", msg->Id_);
		AddMessage_.bindValue (":type", msg->Type_);
		AddMessage_.bindValue (":type_string", msg->TypeString_);
		AddMessage_.bindValue (":read", msg->State_ == LJInbox::MessageState::Read);
		AddMessage_.bindValue (":when", msg->When_);
		if (!AddMessage_.exec ())
		{
			Util::DBLock::DumpError (AddMessage_);
			throw std::runtime_error ("unable to add message");
		}

		AddExtendedMessageParams_.bindValue (":account_id", QString::fromUtf8 (accounId));
		AddExtendedMessageParams_.bindValue (":message_id", msg->Id_);
		AddExtendedMessageParams_.bindValue (":extended_id", msg->ExternalId_);
		AddExtendedMessageParams_.bindValue (":extended_subject", msg->ExtendedSubject_);
		AddExtendedMessageParams_.bindValue (":extended_text", msg->ExtendedText_);
		if (!AddExtendedMessageParams_.exec ())
		{
			Util::DBLock::DumpError (AddExtendedMessageParams_);
			throw std::runtime_error ("unable to add extended message parameters");
		}

		switch (msg->Type_)
		{
		case LJInbox::MessageType::JournalNewComment:
		{
			auto commentMsg = static_cast<LJInbox::MessageNewComment*> (msg);
			AddNewCommentMessageParams_.bindValue (":account_id", QString::fromUtf8 (accounId));
			AddNewCommentMessageParams_.bindValue (":message_id", msg->Id_);
			AddNewCommentMessageParams_.bindValue (":action", commentMsg->Action_);
			AddNewCommentMessageParams_.bindValue (":poster", commentMsg->AuthorName_);
			AddNewCommentMessageParams_.bindValue (":journal", commentMsg->Journal_);
			AddNewCommentMessageParams_.bindValue (":reply_url", commentMsg->ReplyUrl_);
			AddNewCommentMessageParams_.bindValue (":subject", commentMsg->Subject_);
			AddNewCommentMessageParams_.bindValue (":url", commentMsg->Url_);
			if (!AddNewCommentMessageParams_.exec ())
			{
				Util::DBLock::DumpError (AddNewCommentMessageParams_);
				throw std::runtime_error ("unable to add comment message parameters");
			}
			break;
		}
		case LJInbox::MessageType::UserMessageRecvd:
		{
			auto recvdMsg = static_cast<LJInbox::MessageRecvd*> (msg);
			AddReceivedMessageParams_.bindValue (":account_id", QString::fromUtf8 (accounId));
			AddReceivedMessageParams_.bindValue (":message_id", msg->Id_);
			AddReceivedMessageParams_.bindValue (":body", recvdMsg->Body_);
			AddReceivedMessageParams_.bindValue (":from", recvdMsg->From_);
			AddReceivedMessageParams_.bindValue (":subject", recvdMsg->Subject_);
			AddReceivedMessageParams_.bindValue (":recvd_id", recvdMsg->MessageId_);
			AddReceivedMessageParams_.bindValue (":recvd_parent_id", recvdMsg->ParentId_);
			AddReceivedMessageParams_.bindValue (":picture_url", recvdMsg->PictureUrl_);
			if (!AddReceivedMessageParams_.exec ())
			{
				Util::DBLock::DumpError (AddReceivedMessageParams_);
				throw std::runtime_error ("unable to add received message parameters");
			}
			break;
		}
		case LJInbox::MessageType::UserMessageSent:
		{
			auto sentMsg = static_cast<LJInbox::MessageSent*> (msg);
			AddSentMessageParams_.bindValue (":account_id", QString::fromUtf8 (accounId));
			AddSentMessageParams_.bindValue (":message_id", msg->Id_);
			AddSentMessageParams_.bindValue (":body", sentMsg->Body_);
			AddSentMessageParams_.bindValue (":to", sentMsg->To_);
			AddSentMessageParams_.bindValue (":subject", sentMsg->Subject_);
			AddSentMessageParams_.bindValue (":picture_url", sentMsg->PictureUrl_);
			if (!AddSentMessageParams_.exec ())
			{
				Util::DBLock::DumpError (AddSentMessageParams_);
				throw std::runtime_error ("unable to add sent message parameters");
			}
			break;
		}
		default:
			break;
		}
	}

	QList<LJInbox::Message*> LocalStorage::GetAllMessages (const QByteArray& accounId)
	{
		Util::DBLock lock (MetidaDB_);
		lock.Init ();

		GetAllMessages_.bindValue (":account_id", QString::fromUtf8 (accounId));
		if (!GetAllMessages_.exec ())
		{
			Util::DBLock::DumpError (GetAllMessages_);
			throw std::runtime_error ("unable to get all messages");
		}

		QList<LJInbox::Message*> msgs;
		while (GetAllMessages_.next ())
			msgs << GetMessage (GetAllMessages_);

		lock.Good ();

		return msgs;
	}

	QList<LJInbox::Message*> LocalStorage::GetLimitedMessages (int limit,
			int offset, LJInbox::MessageType type, const QByteArray& accounId)
	{
		Util::DBLock lock (MetidaDB_);
		lock.Init ();
		GetLimitedMessages_.bindValue (":account_id", QString::fromUtf8 (accounId));
		GetLimitedMessages_.bindValue (":limit", limit);
		GetLimitedMessages_.bindValue (":offset", offset);
// 		if (type != LJInbox::MessageType::NoType)
// 			GetLimitedMessages_.bindValue (":type", type);

		if (!GetLimitedMessages_.exec ())
		{
			Util::DBLock::DumpError (GetLimitedMessages_);
			throw std::runtime_error ("unable to get limited messages");
		}

		QList<LJInbox::Message*> msgs;
		while (GetLimitedMessages_.next ())
			msgs << GetMessage (GetLimitedMessages_);

		lock.Good ();

		return msgs;
	}

	LJInbox::Message* LocalStorage::GetMessage (int messageId, const QByteArray& accounId)
	{
		Util::DBLock lock (MetidaDB_);
		lock.Init ();

		GetMessage_.bindValue (":account_id", QString::fromUtf8 (accounId));
		GetMessage_.bindValue (":message_id", messageId);

		if (!GetMessage_.exec ())
		{
			Util::DBLock::DumpError (GetMessage_);
			throw std::runtime_error ("unable to get message");
		}

		auto msg = GetMessage_.next () ?
			GetMessage (GetMessage_) :
			0;

		lock.Good ();

		return msg;
	}

	void LocalStorage::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["accounts"] = "CREATE TABLE IF NOT EXISTS accounts ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountID TEXT NOT NULL UNIQUE);";
		table2query ["inbox"] = "CREATE TABLE IF NOT EXISTS inbox ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"MessageId INTEGER NOT NULL UNIQUE, "
				"Type INTEGER NOT NULL, "
				"TypeString TEXT, "
				"Read BOOLEAN, "
				"WhenDate DATE);";
		table2query ["extended"] = "CREATE TABLE IF NOT EXISTS inbox_extended ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE UNIQUE, "
				"ExtendedId INTEGER, "
				"ExtendedSubject TEXT, "
				"ExtendedText TEXT);";
		table2query ["new_comment"] = "CREATE TABLE IF NOT EXISTS inbox_new_comment ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE UNIQUE, "
				"Action TEXT, "
				"Poster TEXT, "
				"Journal TEXT, "
				"ReplyUrl TEXT, "
				"Subject TEXT, "
				"Url TEXT);";
		table2query ["received"] = "CREATE TABLE IF NOT EXISTS inbox_received ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE UNIQUE, "
				"Body TEXT, "
				"Sender TEXT, "
				"Subject TEXT, "
				"ReceivedMessageId INTEGER, "
				"ReceivedMessageParentId INTEGER, "
				"PictureUrl TEXT);";
		table2query ["sent"] = "CREATE TABLE IF NOT EXISTS inbox_sent ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"AccountId INTEGER NOT NULL REFERENCES accounts (Id) ON DELETE CASCADE, "
				"MessageId INTEGER NOT NULL REFERENCES inbox (Id) ON DELETE CASCADE UNIQUE, "
				"Body TEXT, "
				"PictureUrl TEXT, "
				"Receiver TEXT, "
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
				"AccountId, MessageId, Type, TypeString, Read, WhenDate) VALUES ("
				"(SELECT Id FROM accounts WHERE AccountId = :account_id), "
				":message_id, :type, :type_string, :read, :when);");
		RemoveMessage_ = QSqlQuery (MetidaDB_);
		RemoveMessage_.prepare ("DELETE FROM inbox WHERE MessageId = :message_id AND "
				"AccountId = (SELECT Id FROM accounts WHERE AccountId = :account_id);");
		UpdateMessage_ = QSqlQuery (MetidaDB_);
		UpdateMessage_.prepare ("UPDATE inbox SET Read = :read WHERE MessageId = :message_id AND "
				"AccountId = (SELECT Id FROM accounts WHERE AccountId = :account_id);");

		AddExtendedMessageParams_ = QSqlQuery (MetidaDB_);
		AddExtendedMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_extended ("
				"AccountId, MessageId, ExtendedId, ExtendedSubject, ExtendedText) "
				"VALUES ((SELECT Id FROM accounts WHERE AccountId = :account_id), "
				"(SELECT Id FROM inbox WHERE MessageId = :message_id), :extended_id, "
				":extended_subject, :extended_text);");

		AddNewCommentMessageParams_ = QSqlQuery (MetidaDB_);
		AddNewCommentMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_new_comment ("
				"AccountId, MessageId, Action, Poster, Journal, ReplyUrl, Subject, Url) "
				"VALUES ((SELECT Id FROM accounts WHERE AccountId = :account_id), "
				"(SELECT Id FROM inbox WHERE MessageId = :message_id), :action, "
				":poster, :journal, :reply_url, :subject, :url);");

		AddReceivedMessageParams_ = QSqlQuery (MetidaDB_);
		AddReceivedMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_received ("
				"AccountId, MessageId, Body, Sender, Subject, ReceivedMessageId, "
				"ReceivedMessageParentId, PictureUrl) "
				"VALUES ((SELECT Id FROM accounts WHERE AccountId = :account_id), "
				"(SELECT Id FROM inbox WHERE MessageId = :message_id), :body, "
				":from, :journal, :recvd_id, :recvd_parent_id, :picture_url);");

		AddSentMessageParams_ = QSqlQuery (MetidaDB_);
		AddSentMessageParams_.prepare ("INSERT OR IGNORE INTO inbox_sent ("
				"AccountId, MessageId, Body, Receiver, Subject, PictureUrl) "
				"VALUES ((SELECT Id FROM accounts WHERE AccountId = :account_id), "
				"(SELECT Id FROM inbox WHERE MessageId = :message_id), :body, "
				":to, :subject, :picture_url);");

		GetAllMessages_ = QSqlQuery (MetidaDB_);
		GetAllMessages_.prepare ("SELECT * "
				"FROM inbox "
				"LEFT OUTER JOIN inbox_extended ON inbox.AccountId = inbox_extended.AccountId AND "
					"inbox.MessageId = inbox_extended.MessageId "
				"LEFT OUTER JOIN inbox_new_comment  ON inbox.AccountId = inbox_new_comment.AccountId AND "
					"inbox.MessageId = inbox_new_comment.MessageId "
				"LEFT OUTER JOIN inbox_received  ON inbox.AccountId = inbox_received.AccountId AND "
					"inbox.MessageId = inbox_received.MessageId "
				"LEFT OUTER JOIN inbox_sent ON inbox.AccountId = inbox_sent.AccountId AND "
					"inbox.MessageId = inbox_sent.MessageId "
				"WHERE inbox.AccountId = (SELECT Id FROM accounts WHERE AccountId = :accountId);");

		GetLimitedMessages_ = QSqlQuery (MetidaDB_);
		GetLimitedMessages_.prepare ("SELECT * "
				"FROM inbox "
				"LEFT OUTER JOIN inbox_extended ON inbox.AccountId = inbox_extended.AccountId AND "
					"inbox.Id = inbox_extended.MessageId "
				"LEFT OUTER JOIN inbox_new_comment ON inbox.AccountId = inbox_new_comment.AccountId AND "
					"inbox.Id = inbox_new_comment.MessageId "
				"LEFT OUTER JOIN inbox_received ON inbox.AccountId = inbox_received.AccountId AND "
					"inbox.Id = inbox_received.MessageId "
				"LEFT OUTER JOIN inbox_sent ON inbox.AccountId = inbox_sent.AccountId AND "
					"inbox.Id = inbox_sent.MessageId "
				"WHERE inbox.AccountId = (SELECT Id FROM accounts WHERE AccountId = :accountId) "/* AND "
					"inbox.Type = :type OR :type IS NULL "*/
				"LIMIT :limit OFFSET :offset;");

		GetMessage_ = QSqlQuery (MetidaDB_);
		GetMessage_.prepare ("SELECT * "
				"FROM inbox "
				"LEFT OUTER JOIN inbox_extended ON inbox.AccountId = inbox_extended.AccountId AND "
					"inbox.MessageId = inbox_extended.MessageId "
				"LEFT OUTER JOIN inbox_new_comment  ON inbox.AccountId = inbox_new_comment.AccountId AND "
					"inbox.MessageId = inbox_new_comment.MessageId "
				"LEFT OUTER JOIN inbox_received  ON inbox.AccountId = inbox_received.AccountId AND "
					"inbox.MessageId = inbox_received.MessageId "
				"LEFT OUTER JOIN inbox_sent ON inbox.AccountId = inbox_sent.AccountId AND "
					"inbox.MessageId = inbox_sent.MessageId "
				"WHERE inbox.AccountId = (SELECT Id FROM accounts WHERE AccountId = :accountId) "
				"AND inbox.MessageId = (SELECT Id FROM inbox WHERE MessageId = :message_id);");
	}

	void LocalStorage::FillBasicMessage (LJInbox::Message *msg, QSqlQuery getQuery)
	{
		if (!getQuery.isValid ())
			return;

		msg->Id_ = getQuery.value (2).toInt ();
		msg->Type_ = getQuery.value (3).toInt ();
		msg->TypeString_ = getQuery.value (4).toString ();
		msg->State_ = getQuery.value (5).toBool () ?
			LJInbox::MessageState::Read :
			LJInbox::MessageState::UnRead;
		msg->When_ = getQuery.value (6).toDateTime ();
		msg->ExternalId_ = getQuery.value (10).toInt ();
		msg->ExtendedSubject_ = getQuery.value (11).toString ();
		msg->ExtendedText_ = getQuery.value (12).toString ();
	}

	LJInbox::Message* LocalStorage::GetMessage (QSqlQuery getQuery)
	{
		switch (getQuery.value (3).toInt ())
		{
		case LJInbox::MessageType::JournalNewComment:
		{
			LJInbox::MessageNewComment *msg = new LJInbox::MessageNewComment;
			FillBasicMessage (msg, getQuery);
			msg->Action_ = getQuery.value (16).toString ();
			msg->AuthorName_ = getQuery.value (17).toString ();
			msg->Journal_ = getQuery.value (18).toString ();
			msg->ReplyUrl_ = getQuery.value (19).toUrl ();
			msg->Subject_= getQuery.value (20).toString ();
			msg->Url_ = getQuery.value (21).toUrl ();
			return msg;
		}
		case LJInbox::MessageType::UserMessageRecvd:
		{
			LJInbox::MessageRecvd *msg = new LJInbox::MessageRecvd;
			FillBasicMessage (msg, getQuery);
			msg->Body_ = getQuery.value (25).toString ();
			msg->From_ = getQuery.value (26).toString ();
			msg->MessageId_ = getQuery.value (27).toInt ();
			msg->ParentId_ = getQuery.value (28).toInt ();
			msg->PictureUrl_ = getQuery.value (29).toUrl ();
			msg->Subject_ = getQuery.value (30).toString ();
			return msg;
		}
		case LJInbox::MessageType::UserMessageSent:
		{
			LJInbox::MessageSent *msg = new LJInbox::MessageSent;
			FillBasicMessage (msg, getQuery);
			msg->Body_ = getQuery.value (34).toString ();
			msg->PictureUrl_ = getQuery.value (35).toUrl ();
			msg->Subject_ = getQuery.value (36).toString ();
			msg->To_ = getQuery.value (37).toString ();
			break;
		}
		default:
		{
			LJInbox::Message *msg = new LJInbox::Message;
			FillBasicMessage (msg, getQuery);
			return msg;
		}
		}
	}

}
}
}