/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dumper.h"
#include <QtDebug>
#include <util/sll/slotclosure.h>

namespace LC
{
namespace Util
{
	Dumper::Dumper (const QString& from, const QString& to, QObject *parent)
	: QObject { parent }
	, Dumper_ { new QProcess { this } }
	, Restorer_ { new QProcess { this } }
	{
		Iface_.reportStarted ();

		Dumper_->setStandardOutputProcess (Restorer_);

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { HandleProcessError (Dumper_); },
			Dumper_,
			SIGNAL (error (QProcess::ProcessError)),
			Dumper_
		};
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { HandleProcessError (Restorer_); },
			Restorer_,
			SIGNAL (error (QProcess::ProcessError)),
			Restorer_
		};

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { HandleProcessFinished (Dumper_); },
			Dumper_,
			SIGNAL (finished (int, QProcess::ExitStatus)),
			Dumper_
		};
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { HandleProcessFinished (Restorer_); },
			Restorer_,
			SIGNAL (finished (int, QProcess::ExitStatus)),
			Restorer_
		};

		Dumper_->start ("sqlite3", { from, ".dump" });
		Restorer_->start ("sqlite3", { to });
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
			if (HadError_)
				break;

			HadError_ = true;
			ReportResult (tr ("Dumping process crashed: %1.")
					.arg (stderr.isEmpty () ?
							process->errorString () :
							stderr));
			break;
		case QProcess::NormalExit:
			if (exitCode)
				ReportResult (tr ("Dumping process finished with error: %1 (%2).")
						.arg (stderr)
						.arg (exitCode));
			else if (++FinishedCount_ == 2)
			{
				ReportResult (Finished {});
				deleteLater ();
			}
			break;
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

		if (process->error () == QProcess::FailedToStart)
			ReportResult (tr ("Unable to start dumping process: %1. Do you have sqlite3 installed?")
					.arg (process->errorString ()));
		else
			ReportResult (tr ("Unable to dump the database: %1.")
					.arg (process->errorString ()));
	}

	void Dumper::ReportResult (const Result_t& result)
	{
		Iface_.reportFinished (&result);
	}
}
}
