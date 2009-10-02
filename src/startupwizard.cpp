/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "startupwizard.h"
#include "interfaces/istartupwizard.h"
#include "core.h"
#include "application.h"

namespace LeechCraft
{
	StartupWizard::StartupWizard (QWidget *parent)
	: QWizard (parent)
	{
		setWindowTitle (tr ("Startup wizard"));
		setWizardStyle (QWizard::ModernStyle);

		QList<IStartupWizard*> wizards = Core::Instance ()
			.GetPluginManager ()->GetAllCastableTo<IStartupWizard*> ();

		QList<QWizardPage*> pages;
		Q_FOREACH (IStartupWizard *wizard, wizards)
			pages += wizard->GetWizardPages ();

		if (!pages.size ())
		{
			deleteLater ();
			return;
		}

		// Do a queued connection to let all the pages-subscribers to
		// perform the required changes and set up wizard's state.
		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()),
				Qt::QueuedConnection);
		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (handleRejected ()),
				Qt::QueuedConnection);

		Q_FOREACH (QWizardPage *page, pages)
			addPage (page);

		show ();
	}

	void StartupWizard::handleAccepted ()
	{
		if (property ("NeedsRestart").toBool () == true)
			qobject_cast<Application*> (qApp)->InitiateRestart ();

		deleteLater ();
	}

	void StartupWizard::handleRejected ()
	{
		deleteLater ();
	}
};

