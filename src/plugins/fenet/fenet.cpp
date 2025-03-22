/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fenet.h"
#include <QIcon>
#include <QApplication>
#include <QProcess>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "wmfinder.h"
#include "xmlsettingsmanager.h"
#include "compfinder.h"
#include "compparamsmanager.h"
#include "compparamswidget.h"

namespace LC::Fenet
{
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

		Process_ = new QProcess (this);
		connect (Process_,
				SIGNAL (error (QProcess::ProcessError)),
				this,
				SLOT (handleProcessError ()));

		CompProcess_ = new QProcess (this);
		connect (CompProcess_,
				SIGNAL (error (QProcess::ProcessError)),
				this,
				SLOT (handleCompProcessError ()));

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
		KillComp ();
		KillWM ();
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

	void Plugin::KillWM ()
	{
		if (!Process_)
			return;

		Process_->terminate ();
		if (Process_->state () != QProcess::NotRunning && !Process_->waitForFinished (3000))
			Process_->kill ();
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
			return;

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

	void Plugin::KillComp ()
	{
		if (!CompProcess_)
			return;

		CompProcess_->terminate ();
		if (CompProcess_->state () != QProcess::NotRunning && !CompProcess_->waitForFinished (3000))
			CompProcess_->kill ();
	}

	void Plugin::restartWM ()
	{
		KillWM ();
		StartWM ();
	}

	void Plugin::restartComp ()
	{
		KillComp ();
		StartComp ();
	}

	void Plugin::updateCompParamsManager (const QString& name)
	{
		CompParamsManager_->SetCompInfo (GetCompInfo (name));
	}

	void Plugin::handleProcessError ()
	{
		qWarning () << Q_FUNC_INFO
				<< "process error:"
				<< Process_->error ()
				<< Process_->errorString ()
				<< Process_->exitCode ()
				<< Process_->exitStatus ();
	}

	void Plugin::handleCompProcessError ()
	{
		qWarning () << Q_FUNC_INFO
				<< "compositor process error:"
				<< CompProcess_->error ()
				<< CompProcess_->errorString ()
				<< CompProcess_->exitCode ()
				<< CompProcess_->exitStatus ();
	}
}

LC_EXPORT_PLUGIN (leechcraft_fenet, LC::Fenet::Plugin);
