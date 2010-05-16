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
#include <QMessageBox>
#include "interfaces/istartupwizard.h"
#include "core.h"
#include "application.h"
#include "wizardtypechoicepage.h"

namespace LeechCraft
{
	StartupWizard::StartupWizard (QWidget *parent)
	: QWizard (parent)
	, TypeChoseID_ (-10)
	{
		setWindowTitle (tr ("Startup wizard"));
		setWizardStyle (QWizard::ModernStyle);

		QList<IStartupWizard*> wizards = Core::Instance ()
			.GetPluginManager ()->GetAllCastableTo<IStartupWizard*> ();

		Q_FOREACH (IStartupWizard *wizard, wizards)
		{
			QList<QWizardPage*> tp = wizard->GetWizardPages ();
			Pages_ += tp;
		}

		if (!Pages_.size ())
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

		bool hadAdvanced = false;
		bool hadBasic = false;

		Q_FOREACH (QWizardPage *page, Pages_)
			if (page->property ("WizardType").toInt () == StartupWizard::TAdvanced)
				hadAdvanced = true;
			else
				hadBasic = true;

		if (hadBasic && !hadAdvanced)
			AddPages ();
		else if (!hadBasic && hadAdvanced &&
				QMessageBox::question (this,
						"LeechCraft",
						tr ("Would you like to set advanced options?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			AddPages ();
		else
		{
			WizardTypeChoicePage *wtpage = new WizardTypeChoicePage (this);
			TypeChoseID_ = addPage (wtpage);
			AddPages ();
		}

		show ();
	}

	int StartupWizard::nextId () const
	{
		if (TypeChoseID_ < 0 ||
				QWizard::nextId () == -1)
			return QWizard::nextId ();
		else
		{
			WizardTypeChoicePage *wtpage =
					qobject_cast<WizardTypeChoicePage*> (page (TypeChoseID_));
			if (!wtpage)
			{
				qWarning () << Q_FUNC_INFO
						<< "current page is not WizardTypeChoicePage,"
						<< "but it's ID tells us it should be.";
				return -1;
			}

			Type type = wtpage->GetChosenType ();

			const_cast<StartupWizard*> (this)->setProperty ("WizardType", type);

			int nextId = QWizard::nextId ();
			QWizardPage *nextPage = page (nextId);
			QList<QWizardPage*>::const_iterator i =
					std::find (Pages_.begin (), Pages_.end (), nextPage);
			if (i == Pages_.end ())
			{
				qWarning () << Q_FUNC_INFO
						<< "strange, no such page was found"
						<< nextPage
						<< Pages_;
				return -1;
			}
			for (; i != Pages_.end (); ++i)
				if ((*i)->property ("WizardType").toInt () <= type)
					return Page2ID_ [*i];
				else
					disconnect (this,
							0,
							*i,
							0);
			return -1;
		}
	}

	void StartupWizard::AddPages ()
	{
		setProperty ("WizardType", TAdvanced);

		int i = 1;
		Q_FOREACH (QWizardPage *page, Pages_)
		{
			page->setProperty ("PageID", i);
			Page2ID_ [page] = i;
			setPage (i++, page);
		}
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

