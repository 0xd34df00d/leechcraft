/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gdblauncher.h"
#include <algorithm>
#include <utility>
#include <QProcess>
#include <QtDebug>

namespace LC::AnHero::CrashProcess
{
	GDBLauncher::GDBLauncher (quint64 pid, const QString& path, QObject *parent)
	: QObject (parent)
	, Proc_ (new QProcess (this))
	{
		connect (Proc_,
				&QProcess::started,
				this,
				[this]
				{
					Proc_->write ("thread\n");
					Proc_->write ("thread apply all bt\n");
					Proc_->write ("q\n");
				});
		connect (Proc_,
				&QProcess::errorOccurred,
				this,
				[this]
				{
					qDebug () << Q_FUNC_INFO
							<< "status:"
							<< Proc_->exitStatus ()
							<< "code:"
							<< Proc_->exitCode ()
							<< "error:"
							<< Proc_->error ()
							<< "str:"
							<< Proc_->errorString ();

					emit error (Proc_->exitStatus (),
						Proc_->exitCode (),
						Proc_->error (),
						Proc_->errorString ());
				});
		connect (Proc_,
				&QProcess::readyReadStandardOutput,
				this,
				&GDBLauncher::ConsumeStdout);
		connect (Proc_,
				qOverload<int, QProcess::ExitStatus> (&QProcess::finished),
				this,
				[this] (int code, QProcess::ExitStatus status)
				{
					ConsumeStdout ();
					const auto& lastLine = Proc_->readAllStandardOutput ();
					if (!lastLine.isEmpty ())
						HandleLine (lastLine);
					FlushFrame ();

					emit finished (code, status);
				});

		Proc_->start (QStringLiteral ("gdb"),
				{
					"-nw",
					"-n",
					"-q",
					"-p",
					QString::number (pid),
					path
				});
	}

	GDBLauncher::~GDBLauncher ()
	{
		if (Proc_->state () != QProcess::NotRunning)
		{
			Proc_->terminate ();
			if (!Proc_->waitForFinished (500))
				Proc_->kill ();
		}
	}

	void GDBLauncher::ConsumeStdout ()
	{
		while (Proc_->canReadLine ())
			HandleLine (Proc_->readLine ().chopped (1));
	}

	void GDBLauncher::HandleLine (const QByteArray& line)
	{
		const auto& trimmed = line.trimmed ();
		if (!trimmed.isEmpty () && std::ranges::all_of (trimmed, [] (char c) { return c == '.'; }))
			return;

		if (PendingFrame_ && line.startsWith (' ') && line.size () > 1)
		{
			if (!PendingFrame_->endsWith ('('))
				*PendingFrame_ += ' ';
			*PendingFrame_ += trimmed;
			return;
		}

		FlushFrame ();

		if (line.startsWith ('#'))
			PendingFrame_ = trimmed;
		else
			emit gotOutput (QString::fromUtf8 (trimmed));
	}

	void GDBLauncher::FlushFrame ()
	{
		if (const auto frame = std::exchange (PendingFrame_, {}))
			emit gotOutput (QString::fromUtf8 (*frame));
	}
}
