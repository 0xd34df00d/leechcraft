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
	}
}
}
}
