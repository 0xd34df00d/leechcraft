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

#include "bugreportpage.h"
#include <util/sysinfo.h>
#include "reportwizard.h"

namespace LeechCraft
{
namespace Dolozhee
{
	BugReportPage::BugReportPage (ICoreProxy_ptr proxy, QWidget *parent)
	: QWizardPage (parent)
	, Proxy_ (proxy)
	{
		Ui_.setupUi (this);
	}

	int BugReportPage::nextId () const
	{
		return ReportWizard::PageID::Final;
	}

	QString BugReportPage::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QString BugReportPage::GetText () const
	{
		QString result = Ui_.ShortDesc_->toPlainText () + "\n\n";
		result += "*Expected result:*\n" + Ui_.ER_->toPlainText () + "\n\n";
		result += "*Actual result:*\n" + Ui_.AR_->toPlainText () + "\n\n";
		result += "*STR:*\n" + Ui_.STR_->toPlainText () + "\n\n";

		result += "*System information:*\n";
		result += QString ("LeechCraft ") + Proxy_->GetVersion () + "\n";
		result += QString ("Built with Qt %1, running with Qt %2\n")
				.arg (QT_VERSION_STR)
				.arg (qVersion ());
		result += QString ("Running on: %1\n").arg (Util::SysInfo::GetOSName ());

		return result;
	}
}
}
