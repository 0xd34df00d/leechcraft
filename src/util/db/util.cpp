/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QRandomGenerator>
#include <util/sys/paths.h>
#include <util/sll/qtutil.h>
#include "dblock.h"

namespace LC::Util
{
	QSqlQuery RunTextQuery (const QSqlDatabase& db, const QString& text)
	{
		QSqlQuery query { db };
		if (!query.exec (text))
		{
			qDebug () << "unable to execute query";
			DBLock::DumpError (query);
			throw std::runtime_error { "unable to execute query" };
		}
		return query;
	}

	QString LoadQuery (const QString& pluginName, const QString& filename)
	{
		QFile file { ":/" + pluginName + "/resources/sql/" + filename + ".sql" };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< file.fileName ()
					<< file.errorString ();
			throw std::runtime_error { "Cannot open query file" };
		}

		return QString::fromUtf8 (file.readAll ());
	}

	void RunQuery (const QSqlDatabase& db, const QString& pluginName, const QString& filename)
	{
		QSqlQuery query { db };
		query.prepare (LoadQuery (pluginName, filename));
		Util::DBLock::Execute (query);
	}

	QString GenConnectionName (const QString& base)
	{
		return (base + ".%1_%2")
				.arg (QRandomGenerator::global ()->generate ())
				.arg (std::bit_cast<uintptr_t> (QThread::currentThread ()));
	}

	QSqlDatabase OpenSqliteDatabase (const SqliteDatabaseConfig& config)
	{
		auto db = QSqlDatabase::addDatabase ("QSQLITE", Util::GenConnectionName (config.Connection_));
		db.setDatabaseName (Util::GetUserDir (config.DirKind_, config.Dir_).filePath (config.Filename_));
		if (!db.open ())
		{
			qWarning () << "cannot open the database";
			Util::DBLock::DumpError (db.lastError ());
			throw std::runtime_error { "cannot create database" };
		}

		RunTextQuery (db, "PRAGMA synchronous = NORMAL;"_qs);
		RunTextQuery (db, "PRAGMA journal_mode = WAL;"_qs);

		return db;
	}
}
