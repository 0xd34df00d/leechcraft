/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dumper.h"

#include <QCoreApplication>
#include <QProcess>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>

namespace LC::Util
{
	namespace
	{
		struct Tr
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Util::DumpSqlite)
		};

		Either<QString, Void> CheckProcessFinishStatus (QProcess& process)
		{
			const auto& procStdErr = process.readAllStandardError ();
			switch (process.exitStatus ())
			{
			case QProcess::CrashExit:
			{
				const auto& procErr = process.errorString ();

				if (process.error () == QProcess::FailedToStart)
					return Left { Tr::tr ("Unable to start dumping process: %1. Do you have sqlite3 installed?").arg (procErr) };

				const auto& message = procStdErr.isEmpty () ?
						Tr::tr ("Dumping process crashed: %1.").arg (procErr) :
						Tr::tr ("Dumping process crashed: %1 (%2).").arg (procErr, procStdErr);
				return Left { message };
			}
			case QProcess::NormalExit:
				if (const auto ec = process.exitCode ())
				{
					const auto& message = Tr::tr ("Dumping process returned an error: %1 (%2).")
							.arg (ec)
							.arg (procStdErr);
					return Left { message };
				}
				break;
			}

			return Void {};
		}
	}

	Task<Either<QString, Void>> DumpSqlite (QString from, QString to)
	{
		QProcess dumper;
		QProcess restorer;

		dumper.setStandardOutputProcess (&restorer);

		static const auto sqlite = "sqlite3"_qs;
		dumper.start (sqlite, { from, ".dump"_qs });
		restorer.start (sqlite, { to });

		co_await dumper;
		co_await CheckProcessFinishStatus (dumper);

		co_await restorer;
		co_await CheckProcessFinishStatus (restorer);

		co_return Void {};
	}
}
