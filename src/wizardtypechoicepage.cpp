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

#include "wizardtypechoicepage.h"

namespace LeechCraft
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
