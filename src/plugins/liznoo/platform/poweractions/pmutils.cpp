/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pmutils.h"
#include <QProcess>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <util/sll/unreachable.h>
#include <util/threads/futures.h>

namespace LC
{
namespace Liznoo
{
namespace PowerActions
{
	namespace
	{
		QString State2Str (Platform::State state)
		{
			switch (state)
			{
			case Platform::State::Suspend:
				return "suspend";
			case Platform::State::Hibernate:
				return "hibernate";
			}

			Util::Unreachable ();
		}

		QString MakeErrMsg (QProcess *process)
		{
			const auto& origMsg = process->errorString ();
			switch (process->error ())
			{
			case QProcess::ProcessError::FailedToStart:
				return PMUtils::tr ("%1 failed to start. "
						"Probably %2 is not installed? Original message: %3.")
						.arg ("pm-is-supported")
						.arg ("pm-utils")
						.arg (origMsg);
			default:
				return origMsg;
			}
		}
	}

	QFuture<bool> PMUtils::IsAvailable ()
	{
		QFutureInterface<bool> iface;
		iface.reportStarted ();

		auto process = new QProcess { this };
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[process, iface] () mutable
			{
				Util::ReportFutureResult (iface, false);
				process->deleteLater ();
			},
			process,
			SIGNAL (error (QProcess::ProcessError)),
			process
		};
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[process, iface] () mutable
			{
				Util::ReportFutureResult (iface,
						process->exitStatus () == QProcess::NormalExit);
				process->deleteLater ();
			},
			process,
			SIGNAL (finished (int, QProcess::ExitStatus)),
			process
		};
		process->start ("pm-is-supported", QStringList {});

		return iface.future ();
	}

	QFuture<Platform::QueryChangeStateResult> PMUtils::CanChangeState (State state)
	{
		QFutureInterface<QueryChangeStateResult> iface;
		iface.reportStarted ();

		auto process = new QProcess { this };
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[process, iface] () mutable
			{
				const QueryChangeStateResult result { false, MakeErrMsg (process) };
				iface.reportFinished (&result);
				process->deleteLater ();
			},
			process,
			SIGNAL (error (QProcess::ProcessError)),
			process
		};
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[process, iface] () mutable
			{
				const QueryChangeStateResult result { process->exitCode () == 0, {} };
				iface.reportFinished (&result);
				process->deleteLater ();
			},
			process,
			SIGNAL (finished (int, QProcess::ExitStatus)),
			process
		};
		process->start ("pm-is-supported", { "--" + State2Str (state) });

		return iface.future ();
	}

	void PMUtils::ChangeState (State state)
	{
		const auto& app = "pm-" + State2Str (state);

		QProcess::startDetached ("/usr/sbin/" + app, {});
	}
}
}
}

