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

#include "reporttypepage.h"
#include "reportwizard.h"

namespace LeechCraft
{
namespace Dolozhee
{
	ReportTypePage::ReportTypePage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}

	int ReportTypePage::nextId () const
	{
		switch (GetReportType ())
		{
		case Type::Feature:
			return ReportWizard::PageID::FeatureDetails;
		case Type::Bug:
			return ReportWizard::PageID::BugDetails;
		}
	}

	ReportTypePage::Type ReportTypePage::GetReportType () const
	{
		return Ui_.TypeCombo_->currentIndex () == 1 ? Type::Feature : Type::Bug;
	}
}
}
