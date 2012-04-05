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

#include "reportwizard.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QtDebug>
#include <QMessageBox>
#include "chooseuserpage.h"
#include "userstatuspage.h"
#include "reporttypepage.h"
#include "bugreportpage.h"
#include "featurerequestpage.h"
#include "finalpage.h"

namespace LeechCraft
{
namespace Dolozhee
{
	ReportWizard::ReportWizard (ICoreProxy_ptr proxy, QWidget *parent)
	: QWizard (parent)
	, Proxy_ (proxy)
	, NAM_ (new QNetworkAccessManager (this))
	, ChooseUser_ (new ChooseUserPage)
	, ReportType_ (new ReportTypePage)
	, BugReportPage_ (new BugReportPage (proxy))
	, FRPage_ (new FeatureRequestPage)
	, FirstAuth_ (true)
	{
		setWindowTitle (tr ("Issue reporter"));

		setPage (PageID::ChooseUser, ChooseUser_);
		setPage (PageID::UserStatus, new UserStatusPage ());
		setPage (PageID::ReportType, ReportType_);
		setPage (PageID::BugDetails, BugReportPage_);
		setPage (PageID::FeatureDetails, FRPage_);
		setPage (PageID::Final, new FinalPage);

		connect (NAM_,
				SIGNAL (authenticationRequired (QNetworkReply*, QAuthenticator*)),
				this,
				SLOT (handleAuthenticationRequired (QNetworkReply*, QAuthenticator*)));
	}

	QNetworkAccessManager* ReportWizard::GetNAM () const
	{
		return NAM_;
	}

	QNetworkReply* ReportWizard::PostRequest (const QString& address,
			const QByteArray& data)
	{
		QNetworkRequest req ("http://dev.leechcraft.org" + address);
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/xml");
		return NAM_->post (req, data);
	}

	ChooseUserPage* ReportWizard::GetChooseUserPage () const
	{
		return ChooseUser_;
	}

	ReportTypePage* ReportWizard::GetReportTypePage () const
	{
		return ReportType_;
	}

	BugReportPage* ReportWizard::GetBugReportPage () const
	{
		return BugReportPage_;
	}

	FeatureRequestPage* ReportWizard::GetFRPage () const
	{
		return FRPage_;
	}

	void ReportWizard::handleAuthenticationRequired (QNetworkReply*, QAuthenticator *auth)
	{
		qDebug () << Q_FUNC_INFO << FirstAuth_;
		if (FirstAuth_)
		{
			auth->setUser (ChooseUser_->GetLogin ());
			auth->setPassword (ChooseUser_->GetPassword ());
			FirstAuth_ = false;
		}
		else
		{
			QMessageBox::warning (this, "Dolozhee", tr ("Invalid credentials"));
			FirstAuth_ = true;
			restart ();
		}
	}
}
}
