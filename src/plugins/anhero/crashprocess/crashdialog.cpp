/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "crashdialog.h"
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QClipboard>
#include <QProcess>
#include <QTimer>
#include <QtDebug>
#include <util/util.h>
#include <util/gui/util.h>
#include <util/sll/qtutil.h>
#include <util/sys/sysinfo.h>
#include <util/sys/paths.h>
#include "appinfo.h"
#include "gdblauncher.h"
#include "highlighter.h"

namespace LC::AnHero::CrashProcess
{
	namespace
	{
		QString GetNowFilename ()
		{
			const auto& nowStr = QDateTime::currentDateTime ().toString (QStringLiteral ("yy_MM_dd-hh_mm_ss"));
			return "lc_crash_" + nowStr + ".log";
		}
	}

	CrashDialog::CrashDialog (const AppInfo& info, QWidget *parent)
	: QDialog (parent, Qt::Window)
	, CmdLine_ (info.ExecLine_)
	, Info_ (info)
	{
		Ui_.setupUi (this);

		Ui_.InfoLabel_->setText (tr ("Unfortunately LeechCraft has crashed. This is the info we could collect:"));

		QFont traceFont (QStringLiteral ("Terminus"));
		traceFont.setStyleHint (QFont::TypeWriter);
		Ui_.TraceDisplay_->setFont (traceFont);
		Ui_.RestartBox_->setCheckState (info.SuggestRestart_ ? Qt::Checked : Qt::Unchecked);

		new Highlighter (Ui_.TraceDisplay_->document ());

		connect (Ui_.Reload_,
				&QPushButton::released,
				this,
				&CrashDialog::Reload);
		Reload ();

		connect (Ui_.Copy_,
				&QPushButton::released,
				[this]
				{
					const auto& text = Ui_.TraceDisplay_->toPlainText ();
					QGuiApplication::clipboard ()->setText (text, QClipboard::Clipboard);
				});

		connect (Ui_.Save_,
				&QPushButton::released,
				[this]
				{
					const auto& filename = QFileDialog::getSaveFileName (this,
							tr ("Save crash info"),
							QDir::homePath () + "/" + GetNowFilename ());
					if (filename.isEmpty ())
						return;

					WriteTrace (filename);
				});

		connect (this,
				&QDialog::finished,
				[this] (int result)
				{
					if (result == QDialog::Accepted)
					{
						auto reportsDir = Util::CreateIfNotExists (QStringLiteral ("dolozhee/crashreports"));
						const auto& filename = reportsDir.absoluteFilePath (GetNowFilename ());
						WriteTrace (filename);
					}

					auto cmdlist = CmdLine_.split (' ', Qt::SkipEmptyParts);
					cmdlist << QStringLiteral ("--restart");

					if (Ui_.RestartBox_->checkState () == Qt::Checked)
						QProcess::startDetached (Info_.Path_, cmdlist);
				});

		setAttribute (Qt::WA_DeleteOnClose);

		show ();
	}

	void CrashDialog::SetFormat ()
	{
		auto doc = Ui_.TraceDisplay_->document ();
		auto frameFmt = doc->rootFrame ()->frameFormat ();
		frameFmt.setBackground ("#dddddd"_rgb);
		doc->rootFrame ()->setFrameFormat (frameFmt);
	}

	void CrashDialog::WriteTrace (const QString& filename)
	{
		QFile file (filename);
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			QMessageBox::critical (this,
					QStringLiteral ("LeechCraft"),
					tr ("Cannot open file: %1")
						.arg (file.errorString ()));
			return;
		}
		file.write (Ui_.TraceDisplay_->toPlainText ().toUtf8 ());
	}

	void CrashDialog::SetInteractionAllowed (bool allowed)
	{
		Ui_.Reload_->setEnabled (allowed);
		Ui_.Copy_->setEnabled (allowed);
		Ui_.Save_->setEnabled (allowed);
		Ui_.DialogButtons_->button (QDialogButtonBox::Ok)->setEnabled (allowed);
	}

	void CrashDialog::HandleGdbFinished (int code, QProcess::ExitStatus)
	{
		QTimer::singleShot (0, this, [this] { GdbLauncher_.reset (); });

		Ui_.TraceDisplay_->append ("\n\nGDB exited with code " + QString::number (code));
		SetInteractionAllowed (true);

		auto lines = Ui_.TraceDisplay_->toPlainText ().split ('\n');

		const auto pos = std::find_if (lines.begin (), lines.end (),
				[] (const QString& line) { return line.contains ("signal handler called"_ql); });
		if (pos == lines.end ())
			return;

		const auto lastThread = std::find_if (std::reverse_iterator { pos }, std::reverse_iterator { lines.begin () },
				[] (const QString& text) { return text.startsWith ("Thread "_ql); });
		if (lastThread == std::reverse_iterator { lines.begin () })
			return;

		lines.erase (lastThread.base (), pos);

		Ui_.TraceDisplay_->clear ();

		SetFormat ();

		for (const auto& line : lines)
			Ui_.TraceDisplay_->append (line);
	}

	void CrashDialog::Reload ()
	{
		Ui_.TraceDisplay_->clear ();

		SetFormat ();

		GdbLauncher_ = std::make_shared<GDBLauncher> (Info_.PID_, Info_.Path_);
		connect (GdbLauncher_.get (),
				&GDBLauncher::gotOutput,
				Ui_.TraceDisplay_,
				&QTextEdit::append);
		connect (GdbLauncher_.get (),
				&GDBLauncher::finished,
				this,
				&CrashDialog::HandleGdbFinished);
		connect (GdbLauncher_.get (),
				&GDBLauncher::error,
				[this] (QProcess::ExitStatus, int code, QProcess::ProcessError error, const QString& errorStr)
				{
					Ui_.TraceDisplay_->append (QStringLiteral ("\n\nGDB crashed :("));
					Ui_.TraceDisplay_->append (tr ("Exit code: %1; error code: %2; error string: %3.")
							.arg (code)
							.arg (error)
							.arg (errorStr));
				});

		Ui_.TraceDisplay_->append (QStringLiteral ("=== SYSTEM INFO ==="));
		Ui_.TraceDisplay_->append ("Offending signal: " + QString::number (Info_.Signal_));
		Ui_.TraceDisplay_->append ("App path: " + Info_.Path_);
		Ui_.TraceDisplay_->append ("App version: " + Info_.Version_);
		Ui_.TraceDisplay_->append ("Qt version (build-time): " + QStringLiteral (QT_VERSION_STR));
		Ui_.TraceDisplay_->append ("Qt version (runtime): " + QString (qVersion ()));

		const auto& osInfo = Util::SysInfo::GetOSInfo ();
		Ui_.TraceDisplay_->append ("OS: " + osInfo.Name_);
		Ui_.TraceDisplay_->append ("OS version: " + osInfo.Version_);

		Ui_.TraceDisplay_->append (QStringLiteral ("\n\n=== BACKTRACE ==="));

		SetInteractionAllowed (false);
	}
}
