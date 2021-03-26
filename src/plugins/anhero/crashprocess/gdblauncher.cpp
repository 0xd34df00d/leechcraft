/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gdblauncher.h"
#include <stdexcept>
#include <QProcess>
#include <QTextStream>
#include <QtDebug>

namespace LC::AnHero::CrashProcess
{
	GDBLauncher::GDBLauncher (quint64 pid, const QString& path, QObject *parent)
	: QObject (parent)
	, Proc_ (new QProcess (this))
	{
		Proc_->start ("gdb",
				{
					"-nw",
					"-n",
					"-q",
					"-p",
					QString::number (pid),
					path
				});
		connect (Proc_,
				SIGNAL (started ()),
				this,
				SLOT (feedInitialCommands ()));
		connect (Proc_,
				SIGNAL (error (QProcess::ProcessError)),
				this,
				SLOT (handleError ()));
		connect (Proc_,
				SIGNAL (readyReadStandardOutput ()),
				this,
				SLOT (consumeStdout ()));
		connect (Proc_,
				SIGNAL (finished (int, QProcess::ExitStatus)),
				this,
				SIGNAL (finished (int, QProcess::ExitStatus)));
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

	void GDBLauncher::handleError ()
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
	}

	void GDBLauncher::feedInitialCommands ()
	{
		Proc_->write ("thread\n");
		Proc_->write ("thread apply all bt\n");
		Proc_->write ("q\n");
	}

	void GDBLauncher::consumeStdout ()
	{
		auto strs = Proc_->readAllStandardOutput ().trimmed ().split ('\n');
		strs.erase (std::remove_if (strs.begin (), strs.end (),
					[] (const QString& str)
					{
						return !str.isEmpty () &&
								std::all_of (str.begin (), str.end (),
										[] (const QChar& c) { return c == '.'; });
					}),
				strs.end ());

		enum class LineState
		{
			StackFrame,
			Other
		} state = LineState::Other;

		for (auto i = strs.begin (); i != strs.end (); )
		{
			const auto& str = *i;

			if (state == LineState::StackFrame &&
				str.startsWith (' ') && str.size () > 1)
			{
				auto& prevStr = *std::prev (i);
				if (!prevStr.endsWith ('('))
					prevStr += ' ';
				prevStr += str.trimmed ();

				i = strs.erase (i);
			}
			else
			{
				state = str.startsWith ('#') ?
						LineState::StackFrame :
						LineState::Other;
				*i = i->trimmed ();
				++i;
			}
		}

		for (const auto& str : strs)
			emit gotOutput (str);
	}
}
