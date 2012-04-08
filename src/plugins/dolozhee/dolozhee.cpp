/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "dolozhee.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>
#include "reportwizard.h"

namespace LeechCraft
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
		if (area == AEPToolsMenu)
			result << Report_;
		return result;
	}

	void Plugin::initiateReporting ()
	{
		ReportWizard *wizard = new ReportWizard (Proxy_);
		connect (wizard,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (wizard,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
		wizard->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_dolozhee, LeechCraft::Dolozhee::Plugin);
