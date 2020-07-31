/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QDialog>
#include <QProcess>
#include "ui_crashdialog.h"
#include "appinfo.h"

namespace LC
{
namespace AnHero
{
namespace CrashProcess
{
	struct AppInfo;
	class GDBLauncher;

	class CrashDialog : public QDialog
	{
		Q_OBJECT

		Ui::CrashDialog Ui_;
		const QString CmdLine_;
		const AppInfo Info_;

		std::shared_ptr<GDBLauncher> GdbLauncher_;
	public:
		CrashDialog (const AppInfo&, QWidget* = 0);
	private:
		void SetFormat ();
		void WriteTrace (const QString&);
		void SetInteractionAllowed (bool);
	public slots:
		void accept ();
		void done (int);
	private slots:
		void appendTrace (const QString&);
		void handleFinished (int, QProcess::ExitStatus);
		void handleError (QProcess::ExitStatus, int, QProcess::ProcessError, const QString&);

		void reload ();

		void on_Copy__released ();
		void on_Save__released ();
	};
}
}
}
