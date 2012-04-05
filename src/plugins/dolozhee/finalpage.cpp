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

#include "finalpage.h"
#include <QNetworkReply>
#include <QtDebug>
#include <QDomDocument>
#include "reportwizard.h"
#include "reporttypepage.h"
#include "bugreportpage.h"
#include "featurerequestpage.h"
#include "xmlgenerator.h"

namespace LeechCraft
{
namespace Dolozhee
{
	FinalPage::FinalPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}

	void FinalPage::initializePage ()
	{
		auto wiz = qobject_cast<ReportWizard*> (wizard ());

		QString title;
		QString desc;
		const auto type = wiz->GetReportTypePage ()->GetReportType ();
		switch (type)
		{
		case ReportTypePage::Type::Bug:
			title = wiz->GetBugReportPage ()->GetTitle ();
			desc = wiz->GetBugReportPage ()->GetText ();
			break;
		case ReportTypePage::Type::Feature:
			title = wiz->GetFRPage ()->GetTitle ();
			desc = wiz->GetFRPage ()->GetText ();
			break;
		}

		const auto& data = XMLGenerator ().CreateIssue (title, desc, ReportTypePage::Type::Bug);
		auto reply = wiz->PostRequest ("/issues.xml", data);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	void FinalPage::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		QString text;

		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			text = tr ("I'm very sorry to say that, but seems like "
					"we're unable to handle your report at the time :(");
			Ui_.Status_->setText (text);
			return;
		}

		auto root = doc.documentElement ();
		const auto& id = root.firstChildElement ("id").text ();
		text = tr ("Report has been sent successfully. Thanks for your time!");
		if (!id.isEmpty ())
		{
			text += "<br />";
			text += (tr ("Your issue number is %1. You can view it here:") +
						" <a href='http://dev.leechcraft.org/issues/%1'>#%1</a>").arg (id);
		}
		Ui_.Status_->setText (text);
	}
}
}
