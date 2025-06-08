/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "consistencychecker.h"
#include <memory>
#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QtConcurrentRun>
#include <util/gui/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/coro.h>
#include <util/util.h>
#include "dumper.h"
#include "util.h"

namespace LC::Util::ConsistencyChecker
{
	namespace
	{
		struct Tr
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Util::ConsistencyChecker)
		};

		CheckResult_t CheckSync (const QString& dbPath)
		{
			qDebug () << "checking" << dbPath;
			const auto& connName = GenConnectionName ("ConsistencyChecker_" + dbPath);

			auto db = QSqlDatabase::addDatabase ("QSQLITE"_qs, connName);
			const auto remGuard = MakeScopeGuard ([connName] { QSqlDatabase::removeDatabase (connName); });

			db.setDatabaseName (dbPath);
			if (!db.open ())
			{
				qWarning () << "cannot open the DB, but that's not the kind of errors we're solving.";
				return Succeeded {};
			}

			QSqlQuery pragma { db };
			static const auto checkQuery = qgetenv ("LC_THOROUGH_SQLITE_CHECK") == "1" ?
					"PRAGMA integrity_check;"_qs :
					"PRAGMA quick_check;"_qs;
			const auto isGood = pragma.exec (checkQuery) &&
					pragma.next () &&
					pragma.value (0) == "ok";
			qDebug () << "done checking" << dbPath << "; db is good?" << isGood;
			if (isGood)
				return Succeeded {};

			return Left { Failed {} };
		}

		Either<RecoverFailed, Void> CheckRecoverSpace (const QString& dbPath)
		{
			const QFileInfo fi { dbPath };
			const auto filesize = fi.size ();

			const auto available = static_cast<qint64> (GetSpaceInfo (dbPath).Available_);

			qDebug () << "db size:" << filesize
					<< "free space:" << available;
			if (available >= filesize)
				return Void {};

			return Left { RecoverNoSpace { .Available_ = available, .Expected_ = filesize } };
		}
	}

	Task<CheckResult_t> Check (QString dbPath)
	{
		co_return co_await QtConcurrent::run (CheckSync, dbPath);
	}

	Task<RecoverResult_t> Recover (QString dbPath)
	{
		[[maybe_unused]] const auto hasEnoughSpace = co_await CheckRecoverSpace (dbPath);
		const auto& newPath = dbPath + ".new";
		if (QFile::exists (newPath))
			co_return Left { RecoverTargetExists { newPath } };

		const auto dumpProcResult = co_await DumpSqlite (dbPath, newPath);
		[[maybe_unused]] const auto dumpProcSuccess = co_await WithHandler (dumpProcResult,
				[] (const QString& msg) { return RecoverOtherFailure { msg }; });

		const auto oldSize = QFileInfo { dbPath }.size ();
		const auto newSize = QFileInfo { newPath }.size ();

		const auto& backupPath = dbPath + ".bak";
		if (!QFile::rename (dbPath, backupPath))
			co_return Left { RecoverTargetExists { backupPath } };

		// extremely unlikely if the previous rename succeeded
		while (!QFile::rename (newPath, dbPath))
		{
			qCritical () << "unable to rename" << newPath << "→" << dbPath;
			const auto& msg = Tr::tr ("Unable to rename %1 to %2. Please check %2 does not exist, and hit OK.")
					.arg (newPath, dbPath);
			QMessageBox::critical (nullptr, "LeechCraft"_qs, msg);
		}

		co_return RecoverFinished { .OldFileSize_ = oldSize, .NewFileSize_ = newSize };
	}

	namespace
	{
		QString GetRecoverFailureMessage (const RecoverFailed& failure)
		{
			return Visit (failure,
					[&] (RecoverNoSpace space)
					{
						return Tr::tr ("Not enough space available: "
									"%1 free while the restored file is expected to be around %2. "
									"Please either free some disk space on this partition "
									"and retry or cancel the restore process.")
								.arg (MakePrettySize (space.Available_), MakePrettySize (space.Expected_));
					},
					[&] (const RecoverTargetExists& exists)
					{
						return Tr::tr ("Target file %1 already exists, please remove it manually and retry.")
								.arg (FormatName (exists.Target_));
					},
					[&] (const RecoverOtherFailure& other)
					{
						return other.Message_;
					});
		}
	}

	Task<RecoverResult_t> RecoverWithUserInteraction (QString dbPath, QString diaTitle)
	{
		while (true)
		{
			const auto result = co_await Recover (dbPath);
			if (result.IsRight ())
				co_return result;

			const auto& question = Tr::tr ("Unable to dump corrupted SQLite database %1.").arg (FormatName (dbPath)) +
					"<br/><br/>"_qs +
					GetRecoverFailureMessage (result.GetLeft ());
			if (QMessageBox::question (nullptr, diaTitle, question, QMessageBox::Retry | QMessageBox::Cancel) == QMessageBox::Cancel)
				co_return result;
		}
	}
}
