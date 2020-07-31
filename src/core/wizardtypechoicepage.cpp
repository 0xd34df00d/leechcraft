/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "wizardtypechoicepage.h"

namespace LC
{
	WizardTypeChoicePage::WizardTypeChoicePage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
		connect (Ui_.BasicSetup_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleButtonToggled ()));
		connect (Ui_.AdvancedSetup_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleButtonToggled ()));
	}

	WizardTypeChoicePage::~WizardTypeChoicePage ()
	{
	}

	StartupWizard::Type WizardTypeChoicePage::GetChosenType () const
	{
		if (Ui_.BasicSetup_->isChecked ())
			return StartupWizard::TBasic;
		else
			return StartupWizard::TAdvanced;
	}

	void WizardTypeChoicePage::handleButtonToggled ()
	{
		emit chosenTypeChanged (GetChosenType ());
	}
};
