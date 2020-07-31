/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dolozhee.h"
#include <QIcon>
#include <QAction>
#include <QTimer>
#include <util/util.h>
#include <util/sys/paths.h>
#include "reportwizard.h"
#include "reporttypepage.h"
#include "fileattachpage.h"

namespace LC
{
namespace Dolozhee
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("dolozhee");
		Proxy_ = proxy;

		Report_ = new QAction (tr ("Report an issue..."), this);
		Report_->setProperty ("ActionIcon", "tools-report-bug");
		connect (Report_,
				SIGNAL (triggered ()),
				this,
				SLOT (initiateReporting ()));
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (10000,
				this,
				SLOT (checkSavedReports ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Dolozhee";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Dolozhee";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Bug and feature request reporter.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace area) const
	{
		QList<QAction*> result;
		if (area == ActionsEmbedPlace::ToolsMenu)
			result << Report_;
		return result;
	}

	void Plugin::checkSavedReports ()
	{
		const auto& dolozheeDir = Util::CreateIfNotExists ("dolozhee");

		auto news = dolozheeDir;
		if (!news.cd ("crashreports"))
			return;

		news.mkdir ("old");

		QStringList names;
		for (const auto& name : news.entryList (QDir::Files | QDir::NoDotAndDotDot))
		{
			const auto& newName = news.absoluteFilePath ("old/" + name);
			if (!QFile::rename (news.absoluteFilePath (name), newName))
				continue;

			names << newName;
		}

		if (names.isEmpty ())
			return;

		auto wizard = initiateReporting ();
		wizard->GetReportTypePage ()->ForceReportType (ReportTypePage::Type::Bug);

		auto attachPage = wizard->GetFilePage ();
		for (const auto& name : names)
			attachPage->AddFile (name);
	}

	ReportWizard* Plugin::initiateReporting ()
	{
		auto wizard = new ReportWizard (Proxy_);
		wizard->show ();
		return wizard;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_dolozhee, LC::Dolozhee::Plugin);
