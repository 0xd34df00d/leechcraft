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
#include <util/sys/sysinfo.h>
#include <util/sys/paths.h>
#include "appinfo.h"
#include "gdblauncher.h"
#include "highlighter.h"

namespace LC::AnHero::CrashProcess
{
	CrashDialog::CrashDialog (const AppInfo& info, QWidget *parent)
	: QDialog (parent, Qt::Window)
	, CmdLine_ (info.ExecLine_)
	, Info_ (info)
	{
		Ui_.setupUi (this);

		Ui_.InfoLabel_->setText (tr ("Unfortunately LeechCraft has crashed. This is the info we could collect:"));

		QFont traceFont ("Terminus");
		traceFont.setStyleHint (QFont::TypeWriter);
		Ui_.TraceDisplay_->setFont (traceFont);
		Ui_.RestartBox_->setCheckState (info.SuggestRestart_ ? Qt::Checked : Qt::Unchecked);

		new Highlighter (Ui_.TraceDisplay_->document ());

		connect (Ui_.Reload_,
				SIGNAL (released ()),
				this,
				SLOT (reload ()));
		reload ();

		setAttribute (Qt::WA_DeleteOnClose);

		show ();
	}

	void CrashDialog::SetFormat ()
	{
		auto doc = Ui_.TraceDisplay_->document ();
		auto frameFmt = doc->rootFrame ()->frameFormat ();
		frameFmt.setBackground ({ "#dddddd" });
		doc->rootFrame ()->setFrameFormat (frameFmt);
	}

	void CrashDialog::WriteTrace (const QString& filename)
	{
		QFile file (filename);
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			QMessageBox::critical (this,
					"LeechCraft",
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

	namespace
	{
		QString GetNowFilename ()
		{
			const auto& nowStr = QDateTime::currentDateTime ().toString ("yy_MM_dd-hh_mm_ss");
			return "lc_crash_" + nowStr + ".log";
		}
	}

	void CrashDialog::accept ()
	{
		auto reportsDir = Util::CreateIfNotExists ("dolozhee/crashreports");
		const auto& filename = reportsDir.absoluteFilePath (GetNowFilename ());
		WriteTrace (filename);

		QDialog::accept ();
	}

	void CrashDialog::done (int res)
	{
		auto cmdlist = CmdLine_.split (' ', Qt::SkipEmptyParts);
		cmdlist << "--restart";

		if (Ui_.RestartBox_->checkState () == Qt::Checked)
			QProcess::startDetached (Info_.Path_, cmdlist);

		QDialog::done (res);
	}

	void CrashDialog::appendTrace (const QString& part)
	{
		Ui_.TraceDisplay_->append (part);
	}

	namespace
	{
		template<typename T>
		std::reverse_iterator<T> MakeReverse (T t)
		{
			return std::reverse_iterator<T> { t };
		}
	}

	void CrashDialog::handleFinished (int code, QProcess::ExitStatus)
	{
		QTimer::singleShot (0, this, [this] { GdbLauncher_.reset (); });

		Ui_.TraceDisplay_->append ("\n\nGDB exited with code " + QString::number (code));
		SetInteractionAllowed (true);

		auto lines = Ui_.TraceDisplay_->toPlainText ().split ("\n");

		const auto pos = std::find_if (lines.begin (), lines.end (),
				[] (const QString& line) { return line.contains ("signal handler called"); });
		if (pos == lines.end ())
			return;

		const auto lastThread = std::find_if (MakeReverse (pos), MakeReverse (lines.begin ()),
				[] (const QString& text) { return text.startsWith ("Thread "); });
		if (lastThread == MakeReverse (lines.begin ()))
			return;

		lines.erase (lastThread.base (), pos);

		Ui_.TraceDisplay_->clear ();

		SetFormat ();

		for (const auto& line : lines)
			Ui_.TraceDisplay_->append (line);
	}

	void CrashDialog::handleError (QProcess::ExitStatus, int code, QProcess::ProcessError error, const QString& errorStr)
	{
		Ui_.TraceDisplay_->append ("\n\nGDB crashed :(");
		Ui_.TraceDisplay_->append (tr ("Exit code: %1; error code: %2; error string: %3.")
				.arg (code)
				.arg (error)
				.arg (errorStr));
	}

	void CrashDialog::reload ()
	{
		Ui_.TraceDisplay_->clear ();

		SetFormat ();

		GdbLauncher_ = std::make_shared<GDBLauncher> (Info_.PID_, Info_.Path_);
		connect (GdbLauncher_.get (),
				SIGNAL (gotOutput (QString)),
				this,
				SLOT (appendTrace (QString)));
		connect (GdbLauncher_.get (),
				SIGNAL (finished (int, QProcess::ExitStatus)),
				this,
				SLOT (handleFinished (int, QProcess::ExitStatus)));
		connect (GdbLauncher_.get (),
				SIGNAL (error (QProcess::ExitStatus, int, QProcess::ProcessError, QString)),
				this,
				SLOT (handleError (QProcess::ExitStatus, int, QProcess::ProcessError, QString)));

		Ui_.TraceDisplay_->append ("=== SYSTEM INFO ===");
		Ui_.TraceDisplay_->append ("Offending signal: " + QString::number (Info_.Signal_));
		Ui_.TraceDisplay_->append ("App path: " + Info_.Path_);
		Ui_.TraceDisplay_->append ("App version: " + Info_.Version_);
		Ui_.TraceDisplay_->append ("Qt version (build-time): " + QString (QT_VERSION_STR));
		Ui_.TraceDisplay_->append ("Qt version (runtime): " + QString (qVersion ()));

		const auto& osInfo = Util::SysInfo::GetOSInfo ();
		Ui_.TraceDisplay_->append ("OS: " + osInfo.Name_);
		Ui_.TraceDisplay_->append ("OS version: " + osInfo.Version_);

		Ui_.TraceDisplay_->append ("\n\n=== BACKTRACE ===");

		SetInteractionAllowed (false);
	}

	void CrashDialog::on_Copy__released ()
	{
		const auto& text = Ui_.TraceDisplay_->toPlainText ();
		qApp->clipboard ()->setText (text, QClipboard::Clipboard);
	}

	void CrashDialog::on_Save__released ()
	{
		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Save crash info"),
				QDir::homePath () + "/" + GetNowFilename ());

		if (filename.isEmpty ())
			return;

		WriteTrace (filename);
	}
}
