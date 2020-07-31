/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef WIZARDTYPECHOICEPAGE_H
#define WIZARDTYPECHOICEPAGE_H
#include <QWizardPage>
#include "ui_wizardtypechoicepage.h"
#include "startupwizard.h"

namespace LC
{
	class WizardTypeChoicePage : public QWizardPage
	{
		Q_OBJECT

		Ui::WizardTypeChoicePage Ui_;
	public:
		WizardTypeChoicePage (QWidget* = 0);
		virtual ~WizardTypeChoicePage ();

		StartupWizard::Type GetChosenType () const;
	private slots:
		void handleButtonToggled ();
	signals:
		void chosenTypeChanged (StartupWizard::Type);
	};
};

#endif
