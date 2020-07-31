/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bugreportpage.h"
#include <QtDebug>
#include <util/sys/sysinfo.h>
#include "reportwizard.h"

namespace LC
{
namespace Dolozhee
{
	BugReportPage::BugReportPage (ICoreProxy_ptr proxy, QWidget *parent)
	: QWizardPage (parent)
	, Proxy_ (proxy)
	{
		Ui_.setupUi (this);

		connect (Ui_.Title_,
				&QLineEdit::textChanged,
				this,
				&BugReportPage::completeChanged);

		for (auto edit : { Ui_.ShortDesc_, Ui_.AR_, Ui_.STR_ })
			connect (edit,
					&QPlainTextEdit::textChanged,
					this,
					&BugReportPage::completeChanged);
	}

	int BugReportPage::nextId () const
	{
		return ReportWizard::PageID::FilePage;
	}

	bool BugReportPage::isComplete () const
	{
		return !GetTitle ().isEmpty () &&
			!Ui_.ShortDesc_->toPlainText ().isEmpty () &&
			!Ui_.AR_->toPlainText ().isEmpty () &&
			!Ui_.STR_->toPlainText ().isEmpty ();
	}

	QString BugReportPage::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	namespace
	{
		QString GetFormattedVersionString (ICoreProxy_ptr proxy)
		{
			return QString ("LeechCraft ") + proxy->GetVersion () + "\n" +
					QString ("Built with Qt %1, running with Qt %2\n")
							.arg (QT_VERSION_STR)
							.arg (qVersion ()) +
					QString ("Running on: %1\n")
							.arg (Util::SysInfo::GetOSName ());
		}
	}

	QString BugReportPage::GetText () const
	{
		QString result = Ui_.ShortDesc_->toPlainText () + "\n\n";
		result += "*STR:*\n" + Ui_.STR_->toPlainText () + "\n\n";
		result += "*Expected result:*\n" + Ui_.ER_->toPlainText () + "\n\n";
		result += "*Actual result:*\n" + Ui_.AR_->toPlainText () + "\n\n";

		result += "*System information:*\n";
		result += GetFormattedVersionString (Proxy_);

		return result;
	}

	QList<QPair<QString, QString>> BugReportPage::GetReportSections () const
	{
		return
		{
			{ "Short description", Ui_.ShortDesc_->toPlainText () },
			{ "Expected result", Ui_.ER_->toPlainText () },
			{ "Actual result", Ui_.AR_->toPlainText () },
			{ "STR", Ui_.STR_->toPlainText () },
			{ "System information", GetFormattedVersionString (Proxy_) }
		};
	}
}
}
