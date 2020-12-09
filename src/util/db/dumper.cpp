/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dumper.h"
#include <QtDebug>

namespace LC::Util
{
	Dumper::Dumper (const QString& from, const QString& to, QObject *parent)
	: QObject { parent }
	, Dumper_ { new QProcess { this } }
	, Restorer_ { new QProcess { this } }
	{
		Iface_.reportStarted ();

		Dumper_->setStandardOutputProcess (Restorer_);

		connect (Dumper_,
				&QProcess::errorOccurred,
				this,
				[this] { HandleProcessError (Dumper_); });
		connect (Restorer_,
				&QProcess::errorOccurred,
				this,
				[this] { HandleProcessError (Restorer_); });
		connect (Dumper_,
				qOverload<int, QProcess::ExitStatus> (&QProcess::finished),
				this,
				[this] { HandleProcessFinished (Dumper_); });
		connect (Restorer_,
				qOverload<int, QProcess::ExitStatus> (&QProcess::finished),
				this,
				[this] { HandleProcessFinished (Restorer_); });

		static const QString sqliteExecutable = QStringLiteral ("sqlite3");
		Dumper_->start (sqliteExecutable, { from, QStringLiteral (".dump") });
		Restorer_->start (sqliteExecutable, { to });
	}

	QFuture<Dumper::Result_t> Dumper::GetFuture ()
	{
		return Iface_.future ();
	}

	void Dumper::HandleProcessFinished (QProcess *process)
	{
		const auto& stderr = QString::fromUtf8 (process->readAllStandardError ());
		const auto exitCode = process->exitCode ();

		qDebug () << Q_FUNC_INFO
				<< process->exitStatus ()
				<< exitCode
				<< stderr;

		switch (process->exitStatus ())
		{
		case QProcess::CrashExit:
		{
			if (HadError_)
				break;

			HadError_ = true;
			auto errMsg = tr ("Dumping process crashed: %1.")
					.arg (stderr.isEmpty () ?
							process->errorString () :
							stderr);
			ReportResult (Error { std::move (errMsg) });
			break;
		}
		case QProcess::NormalExit:
		{
			if (exitCode)
			{
				auto errMsg = tr ("Dumping process finished with error: %1 (%2).")
						.arg (stderr)
						.arg (exitCode);
				ReportResult (Error { std::move (errMsg) });
			}
			else if (++FinishedCount_ == 2)
			{
				ReportResult (Finished {});
				deleteLater ();
			}
			break;
		}
		}
	}

	void Dumper::HandleProcessError (const QProcess *process)
	{
		qWarning () << Q_FUNC_INFO
				<< process->error ()
				<< process->errorString ();

		if (HadError_)
			return;

		HadError_ = true;

		const auto& errMsg = process->error () == QProcess::FailedToStart ?
				tr ("Unable to start dumping process: %1. Do you have sqlite3 installed?") :
				tr ("Unable to dump the database: %1.");
		ReportResult (Error { errMsg.arg (process->errorString ()) });
	}

	void Dumper::ReportResult (const Result_t& result)
	{
		Iface_.reportFinished (&result);
	}
}
