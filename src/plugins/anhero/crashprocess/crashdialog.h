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

namespace LC::AnHero::CrashProcess
{
	struct AppInfo;
	class GDBLauncher;

	class CrashDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::AnHero::CrashProcess::CrashDialog)

		Ui::CrashDialog Ui_;
		const QString CmdLine_;
		const AppInfo Info_;

		std::shared_ptr<GDBLauncher> GdbLauncher_;
	public:
		explicit CrashDialog (const AppInfo&, QWidget* = nullptr);
	private:
		void SetFormat ();
		void WriteTrace (const QString&);
		void SetInteractionAllowed (bool);

		void Reload ();
		void HandleGdbFinished (int, QProcess::ExitStatus);
	};
}
