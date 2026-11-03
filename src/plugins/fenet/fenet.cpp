/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fenet.h"
#include <csignal>
#include <unistd.h>
#include <QIcon>
#include <QApplication>
#include <QDeadlineTimer>
#include <QProcess>
#include <QSignalBlocker>
#include <util/sys/util.h>
#include <util/threads/coro/context.h>
#include <util/threads/coro/task.h>
#include <util/threads/coro/timer.h>
#include <util/x11/xwrapper.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "wmfinder.h"
#include "xmlsettingsmanager.h"
#include "compfinder.h"
#include "compparamsmanager.h"
#include "compparamswidget.h"

namespace LC::Fenet
{
	namespace
	{
		QProcess* MakeProcess (const char *name, QObject *parent)
		{
			const auto process = new QProcess { parent };
			process->setProcessChannelMode (QProcess::ForwardedChannels);

			const auto leechcraft = getpid ();
			process->setChildProcessModifier ([leechcraft]
					{
						Util::SetParentDeathSignal (SIGTERM);
						// LeechCraft might have exited before the line above
						if (getppid () != leechcraft)
							_exit (1);
					});

			QObject::connect (process,
					&QProcess::errorOccurred,
					process,
					[process, name] (QProcess::ProcessError error)
					{
						qWarning () << name << process->program () << "error:" << error << process->errorString ();
					});
			QObject::connect (process,
					&QProcess::finished,
					process,
					[process, name] (int exitCode, QProcess::ExitStatus status)
					{
						if (status == QProcess::NormalExit)
							qWarning () << name << process->program () << "exited with code" << exitCode;
					});

			return process;
		}

		void Stop (QProcess *process)
		{
			if (!process)
				return;

			const QSignalBlocker expectedExit { process };
			process->terminate ();
			if (process->state () != QProcess::NotRunning && !process->waitForFinished (3000))
			{
				process->kill ();
				process->waitForFinished (1000);
			}
		}

		// the previous manager might still be exiting, like the one of a crashed LeechCraft handling its death signal
		Util::ContextTask<void> StartWhenSelectionFree (Util::CoroContext *request, const char *manager, auto start)
		{
			co_await Util::AddContext { *request };

			if (const auto xwrapper = Util::XWrapper::Instance ())
			{
				using namespace std::chrono_literals;

				const auto selection = xwrapper->GetManagerSelection (manager);
				const auto owner = [&] { return xwrapper->GetSelectionOwner (selection); };

				const QDeadlineTimer warnAt { 5s };
				while (owner () && !warnAt.hasExpired ())
					co_await 100ms;

				if (owner ())
				{
					qWarning () << "the" << manager << "manager selection is still taken, starting once it is free";
					while (owner ())
						co_await 1s;
				}
			}

			start ();
		}
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		Finder_ = new WMFinder;
		CompFinder_ = new CompFinder;
		CompParamsManager_ = new CompParamsManager;

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "fenetsettings.xml");

		XSD_->SetDataSource ("SelectedWM", Finder_->GetFoundModel ());
		XSD_->SetDataSource ("SelectedCompositor", CompFinder_->GetFoundModel ());

		if (!QApplication::arguments ().contains ("--desktop"))
			return;

		Process_ = MakeProcess ("window manager", this);
		CompProcess_ = MakeProcess ("compositor", this);

		StartWM ();
		XmlSettingsManager::Instance ().RegisterObject ("SelectedWM", this, "restartWM");

		XmlSettingsManager::Instance ().RegisterObject ("SelectedCompositor",
				this, "updateCompParamsManager", Util::BaseSettingsManager::EventFlag::Select);
		updateCompParamsManager (XmlSettingsManager::Instance ()
				.property ("SelectedCompositor").toString ());

		StartComp ();
		XmlSettingsManager::Instance ()
			.RegisterObject ({ "SelectedCompositor", "UseCompositor" },
					this, "restartComp");

		connect (CompParamsManager_,
				SIGNAL (paramsChanged ()),
				this,
				SLOT (restartComp ()));

		auto view = new CompParamsWidget ();
		view->setModel (CompParamsManager_->GetModel ());
		XSD_->SetCustomWidget ("CompositorProps", view);

		connect (view,
				SIGNAL (accepted ()),
				CompParamsManager_,
				SLOT (save ()));
		connect (view,
				SIGNAL (rejected ()),
				CompParamsManager_,
				SLOT (revert ()));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Fenet";
	}

	void Plugin::Release ()
	{
		PendingCompStart_.reset ();
		PendingWMStart_.reset ();
		Stop (CompProcess_);
		Stop (Process_);
	}

	QString Plugin::GetName () const
	{
		return "Fenet";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Window manager control plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::StartWM ()
	{
		PendingWMStart_ = std::make_unique<Util::CoroContext> ();
		StartWhenSelectionFree (PendingWMStart_.get (), "WM", [this] { RunSelectedWM (); });
	}

	void Plugin::RunSelectedWM ()
	{
		const auto& found = Finder_->GetFound ();
		if (found.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no known WMs are found, aborting";
			return;
		}

		auto selected = XmlSettingsManager::Instance ()
				.property ("SelectedWM").toString ();

		auto pos = std::find_if (found.begin (), found.end (),
				[&selected] (const WMInfo& info) { return info.Name_ == selected; });
		if (pos == found.end ())
			pos = found.begin ();

		const auto& session = pos->Session_;
		qDebug () << Q_FUNC_INFO << "starting" << session;
		Process_->start (session, QStringList {});
	}

	CompInfo Plugin::GetCompInfo (const QString& selected) const
	{
		const auto& found = CompFinder_->GetFound ();
		if (found.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no known compositors are found, aborting";
			return {};
		}

		auto pos = std::find_if (found.begin (), found.end (),
				[&selected] (const CompInfo& info) { return info.Name_ == selected; });
		if (pos == found.end ())
			pos = found.begin ();

		return *pos;
	}

	void Plugin::StartComp ()
	{
		if (!XmlSettingsManager::Instance ().property ("UseCompositor").toBool ())
		{
			PendingCompStart_.reset ();
			return;
		}

		PendingCompStart_ = std::make_unique<Util::CoroContext> ();
		StartWhenSelectionFree (PendingCompStart_.get (), "_NET_WM_CM", [this] { RunSelectedComp (); });
	}

	void Plugin::RunSelectedComp ()
	{
		auto selected = XmlSettingsManager::Instance ()
				.property ("SelectedCompositor").toString ();
		const auto& info = GetCompInfo (selected);
		if (info.ExecNames_.isEmpty ())
			return;

		const auto& params = CompParamsManager_->GetCompParams (info.Name_);
		const auto& executable = info.ExecNames_.value (0);
		qDebug () << Q_FUNC_INFO << "starting" << executable << params;
		CompProcess_->start (executable, params);
	}

	void Plugin::restartWM ()
	{
		Stop (Process_);
		StartWM ();
	}

	void Plugin::restartComp ()
	{
		Stop (CompProcess_);
		StartComp ();
	}

	void Plugin::updateCompParamsManager (const QString& name)
	{
		CompParamsManager_->SetCompInfo (GetCompInfo (name));
	}
}

LC_EXPORT_PLUGIN (leechcraft_fenet, LC::Fenet::Plugin);
