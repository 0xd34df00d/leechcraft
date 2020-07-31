/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QProcess>

class QProcess;

namespace LC
{
namespace AnHero
{
namespace CrashProcess
{
	class GDBLauncher : public QObject
	{
		Q_OBJECT

		QProcess * const Proc_;
	public:
		GDBLauncher (quint64 pid, const QString& path, QObject* = 0);
		~GDBLauncher ();
	private slots:
		void handleError ();

		void feedInitialCommands ();

		void consumeStdout ();
	signals:
		void gotOutput (const QString&);
		void finished (int, QProcess::ExitStatus);
		void error (QProcess::ExitStatus, int, QProcess::ProcessError, const QString&);
	};
}
}
}
