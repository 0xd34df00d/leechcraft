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

namespace LeechCraft
{
	StartupWizard::StartupWizard (QWidget *parent)
	: QWizard (parent)
	{
		setWindowTitle (tr ("Startup wizard"));
		setAttribute (Qt::WA_DeleteOnClose);

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

		Q_FOREACH (QWizardPage *page, pages)
				addPage (page);

		show ();
	}
};
