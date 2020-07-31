/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "previewpage.h"
#include <QtDebug>
#include <util/sll/unreachable.h>
#include "reportwizard.h"
#include "reporttypepage.h"
#include "bugreportpage.h"
#include "featurerequestpage.h"
#include "xmlgenerator.h"
#include "fileattachpage.h"
#include "chooseuserpage.h"

namespace LC
{
namespace Dolozhee
{
	PreviewPage::PreviewPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		setButtonText (QWizard::NextButton, tr ("Send request"));
	}

	namespace
	{
		QString PriorityToString (ReportTypePage::Priority priority)
		{
			switch (priority)
			{
			case ReportTypePage::Priority::Low:
				return QObject::tr ("Low");
			case ReportTypePage::Priority::Normal:
				return QObject::tr ("Normal");
			case ReportTypePage::Priority::High:
				return QObject::tr ("High");
			}

			Util::Unreachable ();
		}
	}

	void PreviewPage::initializePage ()
	{
		auto wiz = static_cast<ReportWizard*> (wizard ());
		if (!wiz)
			return;

		const auto typePage = wiz->GetReportTypePage ();
		const auto type = typePage->GetReportType ();

		QString title;
		QList<QPair<QString, QString>> sections;
		QString typeText;
		switch (type)
		{
		case ReportTypePage::Type::Bug:
			title = wiz->GetBugReportPage ()->GetTitle ();
			sections = wiz->GetBugReportPage ()->GetReportSections ();
			typeText = tr ("Bug");
			break;
		case ReportTypePage::Type::Feature:
			title = wiz->GetFRPage ()->GetTitle ();
			sections = wiz->GetFRPage ()->GetReportSections ();
			typeText = tr ("Feature");
			break;
		}

		QString preview = "<strong>User:</strong><br/>" +
				((wiz->GetChooseUserPage ()->GetUser () == ChooseUserPage::User::Anonymous) ?
					"Anonymous" :
					wiz->GetChooseUserPage ()->GetLogin ()) + "<br/><br/>";
		preview += "<strong>Title:</strong><br/>" + title + "<br/><br/>";
		preview += "<strong>Type:</strong><br/>" + typeText + "<br/><br/>";
		preview += "<strong>Category:</strong><br/>" + wiz->GetReportTypePage ()->GetCategoryName () + "<br/><br/>";
		preview += "<strong>Priority:</strong><br/>" + PriorityToString (wiz->GetReportTypePage ()->GetPriority ()) + "<br/><br/>";
		for (const auto& section : sections)
			preview += QString ("<strong>%1:</strong><br/>%2<br/><br/>")
					.arg (section.first)
					.arg (section.second.toHtmlEscaped ());
		preview += "<strong>Attached files:</strong><br/>" + wiz->GetFilePage ()->GetFiles ().join ("<br/>");

		preview.remove ("\r");
		preview.replace ("\n", "<br/>");

		Ui_.Preview_->setHtml (preview);
	}

	int PreviewPage::nextId () const
	{
		return ReportWizard::PageID::Final;
	}
}
}
