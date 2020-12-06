/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "consistencychecker.h"
#include <memory>
#include <QFile>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QtConcurrentRun>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include <util/util.h>
#include "dumper.h"
#include "util.h"

namespace LC::Util
{
	class FailedImpl final : public ConsistencyChecker::IFailed
	{
		const std::shared_ptr<ConsistencyChecker> Checker_;
	public:
		explicit FailedImpl (std::shared_ptr<ConsistencyChecker> checker)
		: Checker_ {std::move (checker)}
		{
		}
	private:
		QFuture<ConsistencyChecker::DumpResult_t> DumpReinit () override
		{
			return Checker_->DumpReinit ();
		}
	};

	ConsistencyChecker::ConsistencyChecker (QString dbPath, QString dialogContext, QObject *parent)
	: QObject { parent }
	, DBPath_ { std::move (dbPath) }
	, DialogContext_ { std::move (dialogContext) }
	{
	}

	std::shared_ptr<ConsistencyChecker> ConsistencyChecker::Create (QString dbPath, QString dialogContext)
	{
		return std::shared_ptr<ConsistencyChecker> { new ConsistencyChecker { std::move (dbPath), std::move (dialogContext) } };
	}

	QFuture<ConsistencyChecker::CheckResult_t> ConsistencyChecker::StartCheck ()
	{
		return QtConcurrent::run ([pThis = shared_from_this ()] { return pThis->CheckDB (); });
	}

	ConsistencyChecker::CheckResult_t ConsistencyChecker::CheckDB ()
	{
		qDebug () << Q_FUNC_INFO
				<< "checking"
				<< DBPath_;
		const auto& connName = Util::GenConnectionName ("ConsistencyChecker_" + DBPath_);

		std::shared_ptr<QSqlDatabase> db
		{
			new QSqlDatabase { QSqlDatabase::addDatabase ("QSQLITE", connName) },
			[connName] (QSqlDatabase *db)
			{
				delete db;
				QSqlDatabase::removeDatabase (connName);
			}
		};

		db->setDatabaseName (DBPath_);
		if (!db->open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the DB, but that's not the kind of errors we're solving.";
			return Succeeded {};
		}

		QSqlQuery pragma { *db };
		const QString checkQuery = qgetenv ("LC_THOROUGH_SQLITE_CHECK") == "1" ?
				"PRAGMA integrity_check;" :
				"PRAGMA quick_check;";
		const auto isGood = pragma.exec (checkQuery) &&
				pragma.next () &&
				pragma.value (0) == "ok";
		qDebug () << Q_FUNC_INFO
				<< "done checking"
				<< DBPath_
				<< "; result is:"
				<< isGood;
		if (isGood)
			return Succeeded {};
		else
			return std::make_shared<FailedImpl> (shared_from_this ());
	}

	QFuture<ConsistencyChecker::DumpResult_t> ConsistencyChecker::DumpReinit ()
	{
		QFutureInterface<DumpResult_t> iface;
		iface.reportStarted ();

		DumpReinitImpl (iface);

		return iface.future ();
	}

	namespace
	{
		void ReportResult (QFutureInterface<ConsistencyChecker::DumpResult_t> iface,
				const ConsistencyChecker::DumpResult_t& result)
		{
			iface.reportFinished (&result);
		}
	}

	void ConsistencyChecker::DumpReinitImpl (QFutureInterface<DumpResult_t> iface)
	{
		const QFileInfo fi { DBPath_ };
		const auto filesize = fi.size ();

		while (true)
		{
			const auto available = GetSpaceInfo (DBPath_).Available_;

			qDebug () << Q_FUNC_INFO
					<< "db size:" << filesize
					<< "free space:" << available;
			if (available >= static_cast<quint64> (filesize))
				break;

			if (QMessageBox::question (nullptr,
						DialogContext_,
						tr ("Not enough available space on partition with file %1: "
							"%2 while the restored file is expected to be around %3. "
							"Please either free some disk space on this partition "
							"and retry or cancel the restore process.")
								.arg ("<em>" + DBPath_ + "</em>")
								.arg (Util::MakePrettySize (available))
								.arg (Util::MakePrettySize (filesize)),
						QMessageBox::Retry | QMessageBox::Cancel) == QMessageBox::Cancel)
			{
				ReportResult (iface, DumpError { tr ("Not enough available disk space.") });
				return;
			}
		}

		const auto& newPath = DBPath_ + ".new";

		while (true)
		{
			if (!QFile::exists (newPath))
				break;

			if (QMessageBox::question (nullptr,
						DialogContext_,
						tr ("%1 already exists. Please either remove the file manually "
							"and retry or cancel the restore process.")
								.arg ("<em>" + newPath + "</em>"),
						QMessageBox::Retry | QMessageBox::Cancel) == QMessageBox::Cancel)
			{
				ReportResult (iface, DumpError { tr ("Backup file already exists.") });
				return;
			}
		}

		const auto dumper = new Dumper { DBPath_, newPath };

		const auto managed = shared_from_this ();
		Util::Sequence (nullptr, dumper->GetFuture ()) >>
				[iface, newPath, managed] (const Dumper::Result_t& result)
				{
					Util::Visit (result,
							[iface] (const Dumper::Error& error)
							{
								ReportResult (iface,
										DumpError { tr ("Unable to restore the database.") + " " + error.What_ });
							},
							[iface, newPath, managed] (Dumper::Finished) { managed->HandleDumperFinished (iface, newPath); });
				};
	}

	void ConsistencyChecker::HandleDumperFinished (QFutureInterface<DumpResult_t> iface, const QString& to)
	{
		const auto oldSize = QFileInfo { DBPath_ }.size ();
		const auto newSize = QFileInfo { to }.size ();

		const auto& backup = DBPath_ + ".bak";
		while (!QFile::rename (DBPath_, backup))
			QMessageBox::critical (nullptr,
					DialogContext_,
					tr ("Unable to backup %1 to %2. Please remove %2 and hit OK.")
						.arg (DBPath_)
						.arg (backup));

		QFile::rename (to, DBPath_);

		ReportResult (iface, DumpFinished { oldSize, newSize });
	}
}
