/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "startupwizard.h"
#include <QMessageBox>
#include "interfaces/istartupwizard.h"
#include "core.h"
#include "application.h"
#include "wizardtypechoicepage.h"

namespace LC
{
	StartupWizard::StartupWizard (QWidget *parent)
	: QWizard (parent)
	, TypeChoseID_ (-10)
	, Type_ (TBasic)
	{
		setWindowTitle (tr ("Startup wizard"));
		setWizardStyle (QWizard::ModernStyle);
		handleTypeChanged (Type_);

		QList<IStartupWizard*> wizards = Core::Instance ()
			.GetPluginManager ()->GetAllCastableTo<IStartupWizard*> ();

		for (const auto wizard : wizards)
			Pages_ += wizard->GetWizardPages ();

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

		for (const auto page : Pages_)
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
			connect (wtpage,
					SIGNAL (chosenTypeChanged (StartupWizard::Type)),
					this,
					SLOT (handleTypeChanged (StartupWizard::Type)));
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
			const auto wtpage = qobject_cast<WizardTypeChoicePage*> (page (TypeChoseID_));
			if (!wtpage)
			{
				qWarning () << Q_FUNC_INFO
						<< "current page is not WizardTypeChoicePage,"
						<< "but it's ID tells us it should be.";
				return -1;
			}

			int nextId = QWizard::nextId ();
			auto nextPage = page (nextId);
			auto i = std::find (Pages_.begin (), Pages_.end (), nextPage);
			if (i == Pages_.end ())
			{
				qWarning () << Q_FUNC_INFO
						<< "strange, no such page was found"
						<< nextPage
						<< Pages_;
				return -1;
			}
			for (; i != Pages_.end (); ++i)
				if ((*i)->property ("WizardType").toInt () <= Type_)
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
		for (const auto page : Pages_)
		{
			page->setProperty ("PageID", i);
			Page2ID_ [page] = i;
			setPage (i++, page);
		}
	}

	void StartupWizard::handleTypeChanged (StartupWizard::Type type)
	{
		setProperty ("WizardType", type);
		Type_ = type;
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
}
